// Peter Westerström alternative version of Sean Parents example.
// This one uses std::any and std::function instead to do the type erasure, almost like a
// object-data (in any) + vtable (std::function). We don't get the shared data possibility though.
// Peter Westerström (https://github.com/digimatic), Jan 2021.


#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include <any>
#include <functional>

using namespace std;

template <typename T> void draw(const T& x, ostream& out, size_t position)
{
	out << string(position, ' ') << x << endl;
}

// The alternative object_t implementation.
class object_t
{
public:
	template <typename T>
	object_t(T x)
	    : self_(move(x))
	{
		auto self__ = std::any_cast<T>(&self_);
		draw_ = [self__](ostream& out, size_t position) { draw(*self__, out, position); };
	}

	friend void draw(const object_t& x, ostream& out, size_t position)
	{
		x.draw_(out, position);
	}

private:
	std::any self_;
	std::function<void(ostream& out, size_t position)> draw_;
};

using document_t = vector<object_t>;

void draw(const document_t& x, ostream& out, size_t position)
{
	out << string(position, ' ') << "<document>" << endl;
	for(const auto& e : x)
		draw(e, out, position + 2);
	out << string(position, ' ') << "</document>" << endl;
}

class my_class_t
{
};

void draw(const my_class_t&, ostream& out, size_t position)
{
	out << string(position, ' ') << "my_class_t" << endl;
}
void example1()
{
	document_t document;
	document.emplace_back(0);
	document.emplace_back(string("Hello!"));
	document.emplace_back(document);
	document.emplace_back(my_class_t());
	draw(document, cout, 0);
}

int main()
{
	cout << "Example 1:" << endl;
	example1();
	return 0;
}
