#include <concepts>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <any>

// C++ Type Erasure example in C++ based on the Swift version in:
//  Type Erasure In Swift (Thibault Wittemberg, Dec 5, 2017)
//  https://medium.com/@thibault.wittemberg/type-erasure-in-swift-b561dbaaa152
// Peter Westerström (https://github.com/digimatic), Jan 2021.

// clang-format off
template <typename T>
concept Liquid = requires(T t)
{
	{ t.getTemperature() } ->std::floating_point;
	{ t.getViscosity() } ->std::floating_point;
	{ t.getColor() } ->std::convertible_to<std::string>;
};

template <typename CupType, typename LiquidType>
concept Cup = requires(CupType cup, LiquidType liquid)
{
	{ cup.getLiquid() } ->std::convertible_to<LiquidType>;
	cup.fill(liquid);
};
// clang-format on

// Satisfies concept Liquid
struct Coffee
{
	float temperature;
	auto getViscosity() const
	{
		return 3.4f;
	}
	auto getColor() const -> std::string
	{
		return "black";
	}
	auto getTemperature()
	{
		return temperature;
	}
};

// Satisfies concept Liquid
struct Milk
{
	float temperature;
	auto getViscosity() const
	{
		return 2.2f;
	}
	auto getColor() const -> std::string
	{
		return "white";
	}
	auto getTemperature() const
	{
		return temperature;
	}
};

// Satisfies concept Cup<LiquidType>
template <Liquid LiquidType> struct CeramicCup
{
	LiquidType liquid;
	LiquidType getLiquid() const
	{
		return liquid;
	}
	void fill(LiquidType liquid)
	{
		this->liquid = liquid;
		this->liquid.temperature -= 1.0f;
	}
};
// Satisfies concept Cup<LiquidType>
template <Liquid LiquidType> struct PlasticCup
{
	LiquidType liquid;
	LiquidType getLiquid() const
	{
		return liquid;
	}
	void fill(LiquidType liquid)
	{
		this->liquid = liquid;
		this->liquid.temperature -= 10.0f;
	}
};

template <Liquid LiquidType> struct AbstractCup
{
	virtual ~AbstractCup() = default;
	virtual void fill(LiquidType liquid) = 0;
	virtual LiquidType getLiquid() = 0;
};

template <typename CupType, typename LiquidType>
requires Cup<CupType, LiquidType>&& Liquid<LiquidType> struct CupWrapper
    : public AbstractCup<LiquidType>
{
	CupWrapper(CupType cup)
	    : cup{std::move(cup)}
	{
	}

	void fill(LiquidType liquid) override
	{
		cup.fill(liquid);
	}
	LiquidType getLiquid() override
	{
		return cup.getLiquid();
	}

	CupType cup;
};

#if 0
template <Liquid LiquidType> class AnyCup
{
private:
	std::shared_ptr<AbstractCup<LiquidType>> abstractCup;

public:
	template <typename CupType>
	requires Cup<CupType, LiquidType>
	AnyCup(CupType cup)
	    : abstractCup(std::make_shared<CupWrapper<CupType, LiquidType>>(std::move(cup)))
	{
	}

	void fill(LiquidType liquid)
	{
		abstractCup->fill(liquid);
	}

	LiquidType getLiquid() const
	{
		return abstractCup->getLiquid();
	}
};
#else
template <Liquid LiquidType> class AnyCup
{
private:
	std::any cup;
	std::function<LiquidType()> getLiquidFn;
	std::function<void(LiquidType)> fillFn;

public:
	template <typename CupType>
	requires Cup<CupType, LiquidType>
	AnyCup(CupType cup)
	{
		this->cup = std::move(cup);
		auto cupPtr = std::any_cast<CupType>(&this->cup);

		getLiquidFn = [cupPtr]() { return cupPtr->getLiquid(); };
		fillFn = [cupPtr](LiquidType liquid) mutable { cupPtr->fill(liquid); };
	}

	void fill(LiquidType liquid)
	{
		fillFn(liquid);
	}

	LiquidType getLiquid() const
	{
		return getLiquidFn();
	}
};
#endif
template <typename T> void print(const T& s)
{
	std::cout << s << std::endl;
}

int main()
{
	// Coffee cups
	print("Coffee cups:");
	auto coffeeCups = std::vector<AnyCup<Coffee>>();

	coffeeCups.push_back(AnyCup<Coffee>(CeramicCup<Coffee>()));
	coffeeCups.push_back(AnyCup<Coffee>(PlasticCup<Coffee>()));
	coffeeCups.push_back(PlasticCup<Coffee>()); // AnyCup<..>(..) is not needed

	for(auto& anyCup : coffeeCups)
	{
		anyCup.fill(Coffee{60.4f});
		print(anyCup.getLiquid().getColor());
		print(anyCup.getLiquid().getTemperature());
	}

	// Milk cups
	print("Milk cups:");
	auto milkCups = std::vector<AnyCup<Milk>>();
	milkCups.push_back(AnyCup<Milk>(CeramicCup<Milk>()));
	milkCups.push_back(AnyCup<Milk>(PlasticCup<Milk>()));
	for(auto& anyCup : milkCups)
	{
		anyCup.fill(Milk{30.9f});
		print(anyCup.getLiquid().getColor());
		print(anyCup.getLiquid().getTemperature());
	}

	return 0;
}
