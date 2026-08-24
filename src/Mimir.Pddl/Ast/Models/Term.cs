namespace Mimir.Pddl.Ast.Models;

public sealed record Term : INode
{
    private Term(string name, bool isVariable)
    {
        Name = name;
        IsVariable = isVariable;
    }

    public string Name { get; }

    public bool IsVariable { get; }

    public static Term Variable(string name)
    {
        return new Term(PddlName.RequireVariable(name, nameof(name)), isVariable: true);
    }

    public static Term Constant(string name)
    {
        return new Term(PddlName.RequireName(name, nameof(name)), isVariable: false);
    }

    public string ToPddlString() => Name;

    public override string ToString() => ToPddlString();
}
