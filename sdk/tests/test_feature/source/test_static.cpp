#include "utils.h"

using namespace std;

namespace TestStatic
{

static const char * const TESTNAME = "TestStatic";

bool Test()
{
	bool fail = false;
	int r;
	COutStream out;
	CBufferedOutStream bout;

	// Local static: initialized once, value persists across calls
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"int next()                       \n"
			"{                                \n"
			"  static int v5 = 6;             \n"
			"  return v5++;                   \n"
			"}                                \n"
			"void main()                      \n"
			"{                                \n"
			"  assert(next() == 6);           \n"
			"  assert(next() == 7);           \n"
			"  assert(next() == 8);           \n"
			"}                                \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Local static without initializer (default zero), then assignment
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"int bump()                       \n"
			"{                                \n"
			"  static int n;                  \n"
			"  n = n + 1;                     \n"
			"  return n;                      \n"
			"}                                \n"
			"void main()                      \n"
			"{                                \n"
			"  assert(bump() == 1);           \n"
			"  assert(bump() == 2);           \n"
			"}                                \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Static class members: shared across instances, Class::name access
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"class Counter                    \n"
			"{                                \n"
			"  static int shared = 10;        \n"
			"  void inc() { shared++; }       \n"
			"  int get() { return shared; }   \n"
			"}                                \n"
			"void main()                      \n"
			"{                                \n"
			"  Counter a, b;                  \n"
			"  assert(Counter::shared == 10); \n"
			"  assert(a.get() == 10);         \n"
			"  a.inc();                       \n"
			"  assert(b.get() == 11);         \n"
			"  assert(Counter::shared == 11); \n"
			"  Counter::shared = 20;          \n"
			"  assert(a.get() == 20);         \n"
			"  assert(b.shared == 20);        \n"
			"}                                \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Static class member does not increase instance size vs a similar class without it
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"class WithStatic { int x; static int s; } \n"
			"class Without     { int x; }              \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		else
		{
			asITypeInfo *t1 = mod->GetTypeInfoByName("WithStatic");
			asITypeInfo *t2 = mod->GetTypeInfoByName("Without");
			if( t1 == 0 || t2 == 0 || t1->GetSize() != t2->GetSize() )
				TEST_FAILED;
		}

		engine->Release();
	}

	// Static local inside class method
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"class c                          \n"
			"{                                \n"
			"  int vvv()                      \n"
			"  {                              \n"
			"    static int v = 5;            \n"
			"    return v;                    \n"
			"  }                              \n"
			"}                                \n"
			"void main()                      \n"
			"{                                \n"
			"  c o;                           \n"
			"  assert(o.vvv() == 5);          \n"
			"  assert(o.vvv() == 5);          \n"
			"}                                \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Static local object returned by reference; class with static member
	// (same pattern as returning a reference to a global — static locals are not stack locals)
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"class d                          \n"
			"{                                \n"
			"  int n;                         \n"
			"  static int v = 5;              \n"
			"}                                \n"
			"class c                          \n"
			"{                                \n"
			"  d& ddd()                       \n"
			"  {                              \n"
			"    static d ddd;                \n"
			"    return ddd;                  \n"
			"  }                              \n"
			"}                                \n"
			"void main()                      \n"
			"{                                \n"
			"  c o;                           \n"
			"  assert(d::v == 5);             \n"
			"  o.ddd().n = 1;                 \n"
			"  assert(o.ddd().n == 1);        \n"
			"  c o2;                          \n"
			"  assert(o2.ddd().n == 1);       \n"
			"  assert(o.ddd() is o2.ddd());   \n"
			"  d@ h = o.ddd();                \n"
			"  assert(h is o2.ddd());         \n"
			"  assert(d::v == 5);             \n"
			"}                                \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// disallowGlobalVars: static local must fail with dedicated message
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->SetEngineProperty(asEP_DISALLOW_GLOBAL_VARS, true);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"void f() { static int x = 1; } \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer.find("Global variables are required for 'static'") == string::npos )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// disallowGlobalVars: static class member must fail
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->SetEngineProperty(asEP_DISALLOW_GLOBAL_VARS, true);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"class C { static int x = 1; } \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer.find("Global variables are required for 'static'") == string::npos )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Internal static storage is placed in the internal namespace (hidden from debuggers)
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(TESTNAME,
			"int userGlobal = 1;             \n"
			"class C { static int s = 2; }  \n"
			"int f()                        \n"
			"{                              \n"
			"  static int v = 3;            \n"
			"  return v;                    \n"
			"}                              \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		else
		{
			bool foundUserGlobal = false;
			bool foundInternalStatic = false;
			bool foundUserGlobalInInternalNs = false;

			for( asUINT n = 0; n < mod->GetGlobalVarCount(); n++ )
			{
				const char *name = 0, *ns = 0;
				mod->GetGlobalVar(n, &name, &ns);

				if( ns && strcmp(ns, asINTERNAL_STATIC_NAMESPACE) == 0 )
				{
					if( name && strcmp(name, "userGlobal") == 0 )
						foundUserGlobalInInternalNs = true;
					else if( name && (strncmp(name, "__sl_", 5) == 0 || strncmp(name, "__cm_", 5) == 0) )
						foundInternalStatic = true;
				}
				else if( name && strcmp(name, "userGlobal") == 0 )
					foundUserGlobal = true;
			}

			if( !foundUserGlobal || !foundInternalStatic || foundUserGlobalInInternalNs )
				TEST_FAILED;
		}

		engine->Release();
	}

	// Success
	return fail;
}

} // namespace
