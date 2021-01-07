using System;
using System.Collections.Generic;

interface ILiquid
{
    float Temperature { get; set; }
    float Viscosity { get; }
    string Color { get; }
}

interface ICup<LiquidType>
{
    LiquidType Liquid { get; }
    void Fill(LiquidType liquid);
}

struct Coffee : ILiquid
{
    public float Viscosity { get { return 3.4f; } }
    public string Color { get { return "black"; } }
    public float Temperature { get; set; }
}

struct Milk : ILiquid
{
    public float Viscosity { get { return 2.2f; } }
    public string Color { get { return "white"; } }
    public float Temperature { get; set; }
}

struct CeramicCup<L> : ICup<L> where L : ILiquid
{
    private L liquid;
    public L Liquid
    {
        set { liquid = value; }
        get { return liquid; }
    }
    public void Fill(L liquid)
    {
        Liquid = liquid;
        this.liquid.Temperature = liquid.Temperature - 1.0f;
    }
}

struct PlasticCup<L> : ICup<L> where L : ILiquid
{
    private L liquid;
    public L Liquid
    {
        set { liquid = value; }
        get { return liquid; }
    }
    public void Fill(L liquid)
    {
        Liquid = liquid;
        this.liquid.Temperature = liquid.Temperature - 10.0f;
    }
}

class CupFiller<L>
    where L : ILiquid, new()
{
    public static void FillCup(ref ICup<L> cup, float temp)
    {
        var c = new L();
        c.Temperature = temp;
        cup.Fill(c);
    }
}

// No need to type erase to have a List<ICup<Coffee>>. But if we go one step
// further and want to have a List<ICup> (any cup, any liquid). We have to
// type erase the liquid type.
interface ICup : ICup<ILiquid>
{
}

struct CupWrapper<L> : ICup
    where L : struct, ILiquid
{
    public CupWrapper(ICup<L> cup)
    {
        this.cup = cup;
    }

    readonly ICup<L> cup;

    public ILiquid Liquid
    {
        get
        {
            return cup.Liquid;
        }
    }

    public void Fill(ILiquid liquid)
    {
        bool b = liquid is L;
        if (b)
        {
            var l = (L)liquid;
            cup.Fill(l);
        }
    }
}

struct AnyCup : ICup
{
    public AnyCup(ICup cup)
    {
        this.cup = cup;
    }

    public static AnyCup CreateAnyCup<L>(ICup<L> cup)
        where L : struct, ILiquid
    {
        var cupWrap = new CupWrapper<L>(cup);
        var anyCup = new AnyCup(cupWrap);
        return anyCup;
    }

    ICup cup;
    public ILiquid Liquid
    {
        get
        {
            return cup.Liquid;
        }

    }

    public void Fill(ILiquid liquid)
    {
        cup.Fill(liquid);
    }
}

namespace typeerasureexamplecsharp
{
    class MainClass
    {
        public static void Main(string[] args)
        {
            var cupsOfCoffee = new List<ICup<Coffee>>();
            cupsOfCoffee.Add(new CeramicCup<Coffee>());
            cupsOfCoffee.Add(new PlasticCup<Coffee>());

            foreach (var cup in cupsOfCoffee)
            {
                var c = new Coffee();
                c.Temperature = 60.4f;
                cup.Fill(c);
                Console.WriteLine($"{cup.Liquid.Color}");
                Console.WriteLine($"{cup.Liquid.Temperature}");
            }

            //
            // Next level, needs type erasure in C# too
            //
            {
                var anyCups = new List<ICup>();
                ICup cup = AnyCup.CreateAnyCup(new CeramicCup<Coffee>());
                var hotCoffee = new Coffee();
                hotCoffee.Temperature = 90.0f;
                cup.Fill(hotCoffee);
                anyCups.Add(cup);
            }
            //            anyCups.Add(new CeramicCup<Milk>());
            //            anyCups.Add(new PlasticCup<Coffee>());

        }
    }
}
