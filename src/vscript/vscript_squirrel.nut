static char g_Script_vscript_squirrel[] = R"vscript(
//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose: 
// 
//=============================================================================//

Warning <- error;

function clamp(val,min,max)
{
	if ( max < min )
		return max;
	else if( val < min )
		return min;
	else if( val > max )
		return max;
	else
		return val;
}

function max(a,b) return a > b ? a : b

function min(a,b) return a < b ? a : b

function RemapVal(val, A, B, C, D)
{
	if ( A == B )
		return val >= B ? D : C;
	return C + (D - C) * (val - A) / (B - A);
}

function RemapValClamped(val, A, B, C, D)
{
	if ( A == B )
		return val >= B ? D : C;
	local cVal = (val - A) / (B - A);
	cVal = (cVal < 0.0) ? 0.0 : (1.0 < cVal) ? 1.0 : cVal;
	return C + (D - C) * cVal;
}

function Approach( target, value, speed )
{
	local delta = target - value

	if( delta > speed )
		value += speed
	else if( delta < (-speed) )
		value -= speed
	else
		value = target

	return value
}

function AngleDistance( next, cur )
{
	local delta = next - cur

	if ( delta < (-180.0) )
		delta += 360.0
	else if ( delta > 180.0 )
		delta -= 360.0

	return delta
}

function FLerp( f1, f2, i1, i2, x )
{
	return f1+(f2-f1)*(x-i1)/(i2-i1);
}

function Lerp( f, A, B )
{
	return A + ( B - A ) * f
}

function SimpleSpline( f )
{
	local ff = f * f;
	return 3.0 * ff - 2.0 * ff * f;
}

function printl( text )
{
	return ::print(text + "\n");
}

class CSimpleCallChainer
{
	constructor(prefixString, scopeForThis, exactMatch)
	{
		prefix = prefixString;
		scope = scopeForThis;
		chain = [];
		scope["Dispatch" + prefixString] <- Call.bindenv(this);
	}

	function PostScriptExecute()
	{
		local func;
		try {
			func = scope[prefix];
		} catch(e) {
			return;
		}
		if (typeof(func) != "function")
			return;
		chain.push(func);
	}

	function Call()
	{
		foreach (func in chain)
		{
			func.pcall(scope);
		}
	}

	prefix = null;
	scope = null;
	chain = null;
}

__Documentation <- {}

local DocumentedFuncs   = {}
local DocumentedClasses = {}
local DocumentedEnums   = {}
local DocumentedConsts  = {}
local DocumentedHooks   = {}
local DocumentedMembers = {}

local function AddAliasedToTable(name, signature, description, table)
{
	// This is an alias function, could use split() if we could guarantee
	// that ':' would not occur elsewhere in the description and Squirrel had
	// a convience join() function -- It has split()
	local colon = description.find(":");
	if (colon == null)
		colon = description.len();
	local alias = description.slice(1, colon);
	description = description.slice(colon + 1);
	name = alias;
	signature = null;

	table[name] <- [signature, description];
}

function __Documentation::RegisterHelp(name, signature, description)
{
	if (description.len() && description[0] == '#')
	{
		AddAliasedToTable(name, signature, description, DocumentedFuncs)
	}
	else
	{
		DocumentedFuncs[name] <- [signature, description];
	}
}

function __Documentation::RegisterClassHelp(name, baseclass, description)
{
	DocumentedClasses[name] <- [baseclass, description];
}

function __Documentation::RegisterEnumHelp(name, num_elements, description)
{
	DocumentedEnums[name] <- [num_elements, description];
}

function __Documentation::RegisterConstHelp(name, signature, description)
{
	if (description.len() && description[0] == '#')
	{
		AddAliasedToTable(name, signature, description, DocumentedConsts)
	}
	else
	{
		DocumentedConsts[name] <- [signature, description];
	}
}

function __Documentation::RegisterHookHelp(name, signature, description)
{
	DocumentedHooks[name] <- [signature, description];
}

function __Documentation::RegisterMemberHelp(name, signature, description)
{
	DocumentedMembers[name] <- [signature, description];
}

local function printdoc( text )
{
	return ::printc(200,224,255,text);
}

local function printdocl( text )
{
	return printdoc(text + "\n");
}

local function PrintClass(name, doc)
{
	local text = "=====================================\n";
	text += ("Class:       " + name + "\n");
	text += ("Base:        " + doc[0] + "\n");
	if (doc[1].len())
		text += ("Description: " + doc[1] + "\n");
	text += "=====================================\n\n";

	printdoc(text);
}

local function PrintFunc(name, doc)
{
	local text = "Function:    " + name + "\n"

	if (doc[0] == null)
	{
		// Is an aliased function
		text += ("Signature:   function " + name + "(");
		foreach(k,v in this[name].getinfos().parameters)
		{
			if (k == 0 && v == "this") continue;
			if (k > 1) text += (", ");
			text += (v);
		}
		text += (")\n");
	}
	else
	{
		text += ("Signature:   " + doc[0] + "\n");
	}
	if (doc[1].len())
		text += ("Description: " + doc[1] + "\n");
	printdocl(text);
}

local function PrintMember(name, doc)
{
	local text = ("Member:      " + name + "\n");
	text += ("Signature:   " + doc[0] + "\n");
	if (doc[1].len())
		text += ("Description: " + doc[1] + "\n");
	printdocl(text);
}

local function PrintEnum(name, doc)
{
	local text = "=====================================\n";
	text += ("Enum:        " + name + "\n");
	text += ("Elements:    " + doc[0] + "\n");
	if (doc[1].len())
		text += ("Description: " + doc[1] + "\n");
	text += "=====================================\n\n";

	printdoc(text);
}

local function PrintConst(name, doc)
{
	local text = ("Constant:    " + name + "\n");
	if (doc[0] == null)
	{
		text += ("Value:       null\n");
	}
	else
	{
		text += ("Value:       " + doc[0] + "\n");
	}
	if (doc[1].len())
		text += ("Description: " + doc[1] + "\n");
	printdocl(text);
}

local function PrintHook(name, doc)
{
	local text = ("Hook:        " + name + "\n");
	if (doc[0] == null)
	{
		// Is an aliased function
		text += ("Signature:   function " + name + "(");
		foreach(k,v in this[name].getinfos().parameters)
		{
			if (k == 0 && v == "this") continue;
			if (k > 1) text += (", ");
			text += (v);
		}
		text += (")\n");
	}
	else
	{
		text += ("Signature:   " + doc[0] + "\n");
	}
	if (doc[1].len())
		text += ("Description: " + doc[1] + "\n");
	printdocl(text);
}

local function PrintMatchesInDocList(pattern, list, printfunc)
{
	local foundMatches = 0;

	foreach(name, doc in list)
	{
		if (pattern == "*" || name.tolower().find(pattern) != null || (doc[1].len() && doc[1].tolower().find(pattern) != null))
		{
			foundMatches = 1;
			printfunc(name, doc)
		}
	}

	return foundMatches;
}

function __Documentation::PrintHelp(pattern = "*")
{
	local patternLower = pattern.tolower();

	// Have a specific order
	if (!(
		PrintMatchesInDocList( patternLower, DocumentedEnums, PrintEnum )		|
		PrintMatchesInDocList( patternLower, DocumentedConsts, PrintConst )		|
		PrintMatchesInDocList( patternLower, DocumentedClasses, PrintClass )	|
		PrintMatchesInDocList( patternLower, DocumentedFuncs, PrintFunc )		|
		PrintMatchesInDocList( patternLower, DocumentedMembers, PrintMember )	|
		PrintMatchesInDocList( patternLower, DocumentedHooks, PrintHook )
	   ))
	{
		printdocl("Pattern " + pattern + " not found");
	}
}

// Vector documentation
__Documentation.RegisterClassHelp( "Vector", "", "Basic 3-float Vector class." );
__Documentation.RegisterHelp( "Vector::Length", "float Vector::Length()", "Return the vector's length." );
__Documentation.RegisterHelp( "Vector::LengthSqr", "float Vector::LengthSqr()", "Return the vector's squared length." );
__Documentation.RegisterHelp( "Vector::Length2D", "float Vector::Length2D()", "Return the vector's 2D length." );
__Documentation.RegisterHelp( "Vector::Length2DSqr", "float Vector::Length2DSqr()", "Return the vector's squared 2D length." );

__Documentation.RegisterHelp( "Vector::Normalized", "float Vector::Normalized()", "Return a normalized version of the vector." );
__Documentation.RegisterHelp( "Vector::Norm", "void Vector::Norm()", "Normalize the vector in place." );
__Documentation.RegisterHelp( "Vector::Scale", "vector Vector::Scale(float)", "Scale the vector's magnitude and return the result." );
__Documentation.RegisterHelp( "Vector::Dot", "float Vector::Dot(vector)", "Return the dot/scalar product of two vectors." );
__Documentation.RegisterHelp( "Vector::Cross", "float Vector::Cross(vector)", "Return the vector product of two vectors." );

__Documentation.RegisterHelp( "Vector::ToKVString", "string Vector::ToKVString()", "Return a vector as a string in KeyValue form, without separation commas." );

__Documentation.RegisterMemberHelp( "Vector.x", "float Vector.x", "The vector's X coordinate on the cartesian X axis." );
__Documentation.RegisterMemberHelp( "Vector.y", "float Vector.y", "The vector's Y coordinate on the cartesian Y axis." );
__Documentation.RegisterMemberHelp( "Vector.z", "float Vector.z", "The vector's Z coordinate on the cartesian Z axis." );

)vscript";