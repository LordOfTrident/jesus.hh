#define JESUS_HH__IMPLEMENTATION__
#include "jesus.hh"

using namespace Jesus;

void HelloWorld() {
	Stdout.PrintLn("Hello, world!");
	Stdout.Print("Hello, world!\n");

	Stdout.PrintLn("Hello, %v!"_s % "world");
	Stdout.PrintLn("Hello, ", "world", "!");

	Stdout.Write("Hello, world!\n");
}

void Strings() {
	StringView sv("the trident");
	Stdout.PrintLn("'%v'"_s % sv.Raw());

	Stdout.PrintLn(sv.Prefix("the")? "- Starts with 'the'" : "- Does not start with 'the'");
	Stdout.PrintLn(sv.Suffix("ent")? "- Ends with 'ent'" : "- Does not end with 'ent'");
	Stdout.PrintLn("- 't' occurs ", sv.Occurs('t'), " times");
	Stdout.PrintLn("- Sub(4, 3) == '", sv.Sub(4, 3), "'");

	auto res = sv.FirstOf("hello");
	if (res.none)
		Stdout.PrintLn("- does not contain 'hello'");
	else
		Stdout.PrintLn("- contains 'hello' at ", res.unwrap);
}

int main() {
	HelloWorld();
	Stdout.PrintLn();
	Strings();

	return EXIT_OK;
}
