#include <iostream>
#include <llvm-c/Core.h>


#include "../parsing/lexer.hh"
#include "../core/treeprinter.hh"
#include "../codegen/moduleCompiler.hh"

// #include "../passes/InferTypesPass.cc"
// #include "../passes/ReturnInsertPass.cc"
// #include "../passes/UnclassifyPass.cc"
// #include "../passes/MainFuncPass.cc"

using namespace coral;
using namespace std;

void testInferTypes();

// int main() {
//   testInferTypes();
//   return 0;

//   cout << "---- [ Codegen Test ] ----\n";
//   auto m = parse(0, R"CORAL(
// extern "C" printf : Fn[..., Void]

// func main ():
//   printf "Hello World\n"

// )CORAL");
//   m->name = "codegen-test";
//   m = parse(fopen("tests/libs/array.coral", "r"), R"CORAL(
// # This is the synchronous input / output module
// # We should really be using async for everything in the long run though,
// # especially in core so we're not wasting thread time
// type platform:
//  extern "C" read  : Fn[Int32, Ptr[Int8], Int32]
//  extern "C" write : Fn[Int32, Ptr[Int8], Int32]

// type FdByteReader:
//   let fd: Int32
//   func read(n):
//     # TODO buffer input
//     let buf = Array[Int8].create n
//     let m = platform.read(fd, buf, intNative n)
//     ByteString.new(buf, m)

// # type FdByteWriter:
// #   let fd: Int32
// #   func write(bs: ByteString):
// #     platform.write(fd, bs.buf, bs.length)

// # type StringFromByteReader(ByteReaderT):
// #   let bytes: ByteReaderT
// #   func new(fd):
// #     set bytes = ByteReaderT(fd)
// #   func new(readbytes):
// #     set bytes = readbytes
// #   func read(n):
// #     TextString.new(bytes.read(n))

// # type FdStringReader = StringFromByteReader(FdByteReader)
// # type FdStringWriter = StringFromByteWriter(FdByteWriter)

// # _stdin = FdByteReader(0)
// # _stdout = FdByteWriter(1)
// # _stderr = FdByteWriter(2)
// # stdin = FdStringReader(_stdin)
// # stdout = FdStringWriter(_stdout)
// # stderr = FdStringWriter(_stderr)

// # stdout.write("[test]\n")
// # stderr.write("[foobar]\n")

// )CORAL");
//   return 0;
//   m = parse(fopen("tests/libs/array.coral", "r"), 0);
//   m = (Module *)InferTypesPass(m).out;
//   m = (Module *)ReturnInsertPass(m).out;
//   m = (Module *)MainFuncPass(m).out;
//   TreePrinter(m, cout).print();
//   cout << ModuleBuilder(m).finalize();
// }

void jit_modules(std::vector<Module *> modules);

int main() {
  auto m = parse(fopen("tests/codegen/wip.coral", "r"), 0);
  coral::ModuleCompiler mc (m);
  cout << "++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  cout << mc.getIR();
  // jit_modules(std::vector<Module *> { m });
  // auto m = parse(fopen("tests/libs/array.coral", "r"), 0);
  // m = UnclassifyPass(m).module;
  // TreePrinter(m, cout).print();

}
