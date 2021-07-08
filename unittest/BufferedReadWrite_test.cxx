/**
 * @file BufferedReadWrite_test.cxx Unit Tests for BufferedFileWriter and BufferedFileReader
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

/**
 * @brief Name of this test module
 */
#define BOOST_TEST_MODULE BufferedReadWrite_test // NOLINT

#include "boost/test/unit_test.hpp"

#include "logging/Logging.hpp"
#include "readout/ReadoutTypes.hpp"
#include "readout/utils/BufferedFileReader.hpp"
#include "readout/utils/BufferedFileWriter.hpp"

#include <cstdio>
#include <string>
#include <vector>

using namespace dunedaq::readout;

BOOST_AUTO_TEST_SUITE(BufferedReadWrite_test)

void
test_read_write(BufferedFileWriter<int>& writer, BufferedFileReader<int>& reader, uint numbers_to_write)
{
  std::vector<int> numbers(numbers_to_write / sizeof(int));

  bool write_successful = false;
  for (uint i = 0; i < numbers.size(); ++i) {
    numbers[i] = i;
    write_successful = writer.write(i);
    if (!write_successful)
      BOOST_REQUIRE(write_successful);
  }

  writer.close();

  int read_value;
  bool read_successful = false;
  for (uint i = 0; i < numbers.size(); ++i) {
    read_successful = reader.read(read_value);
    if (!read_successful) {
      TLOG() << i;
      BOOST_REQUIRE(read_successful);
    }
    if (read_value != numbers[i]) {
      BOOST_REQUIRE_EQUAL(read_value, numbers[i]);
    }
  }

  read_successful = reader.read(read_value);
  BOOST_REQUIRE(!read_successful);

  reader.close();

  remove("test.out");
}

BOOST_AUTO_TEST_CASE(BufferedReadWrite_one_int)
{
  TLOG() << "Trying to write and read one int" << std::endl;
  remove("test.out");
  BufferedFileWriter<int> writer;
  writer.open("test.out", 4096);
  BufferedFileReader<int> reader;
  reader.open("test.out", 4096);
  uint numbers_to_write = 1;

  test_read_write(writer, reader, numbers_to_write);
}

BOOST_AUTO_TEST_CASE(BufferedReadWrite_extended)
{
  TLOG() << "Trying to read and write more ints" << std::endl;
  remove("test.out");
  BufferedFileWriter<int> writer;
  writer.open("test.out", 4096);
  BufferedFileReader<int> reader;
  reader.open("test.out", 4096);
  uint numbers_to_write = 4096 * 4096;

  test_read_write(writer, reader, numbers_to_write);
}

BOOST_AUTO_TEST_CASE(BufferedReadWrite_zstd)
{
  TLOG() << "Testing zstd compression" << std::endl;
  remove("test.out");
  BufferedFileWriter<int> writer;
  writer.open("test.out", 4096, "zstd");
  BufferedFileReader<int> reader;
  reader.open("test.out", 4096, "zstd");
  uint numbers_to_write = 4096 * 4096;

  test_read_write(writer, reader, numbers_to_write);
}

BOOST_AUTO_TEST_CASE(BufferedReadWrite_lzma)
{
  TLOG() << "Testing lzma compression" << std::endl;
  remove("test.out");
  BufferedFileWriter<int> writer;
  writer.open("test.out", 4096, "lzma");
  BufferedFileReader<int> reader;
  reader.open("test.out", 4096, "lzma");
  uint numbers_to_write = 4096 * 4096;

  test_read_write(writer, reader, numbers_to_write);
}

BOOST_AUTO_TEST_CASE(BufferedReadWrite_zlib)
{
  TLOG() << "Testing zlib compression" << std::endl;
  remove("test.out");
  BufferedFileWriter<int> writer;
  writer.open("test.out", 4096, "zlib");
  BufferedFileReader<int> reader;
  reader.open("test.out", 4096, "zlib");
  uint numbers_to_write = 4096 * 4096;

  test_read_write(writer, reader, numbers_to_write);
}

BOOST_AUTO_TEST_CASE(BufferedReadWrite_not_opened)
{
  TLOG() << "Try to read and write on uninitialized instances" << std::endl;
  BufferedFileWriter<int> writer;
  bool write_successful = writer.write(42);
  BOOST_REQUIRE(!write_successful);

  BufferedFileReader<int> reader;
  int value;
  bool read_successful = reader.read(value);
  BOOST_REQUIRE(!read_successful);
}

BOOST_AUTO_TEST_CASE(BufferedReadWrite_already_closed)
{
  TLOG() << "Try to write on closed writer" << std::endl;
  BufferedFileWriter<int> writer("test.out", 4096);
  writer.close();
  bool write_successful = writer.write(42);
  BOOST_REQUIRE(!write_successful);
}

BOOST_AUTO_TEST_CASE(BufferedReadWrite_destructor)
{
  TLOG() << "Testing automatic closing on destruction" << std::endl;
  remove("test.out");
  {
    BufferedFileWriter<int> writer("test.out", 4096);
    bool write_successful = writer.write(42);
    BOOST_REQUIRE(write_successful);
  }
  BufferedFileReader<int> reader("test.out", 4096);
  int value;
  bool read_successful = reader.read(value);
  BOOST_REQUIRE(read_successful);
  BOOST_REQUIRE_EQUAL(value, 42);

  reader.close();

  remove("test.out");
}

BOOST_AUTO_TEST_CASE(BufferedReadWrite_superchunk)
{
  remove("test.out");
  TLOG() << "Read and write superchunks" << std::endl;
  BufferedFileWriter<types::WIB_SUPERCHUNK_STRUCT> writer("test.out", 8388608);

  std::vector<types::WIB_SUPERCHUNK_STRUCT> chunks(100000);
  for (uint i = 0; i < chunks.size(); ++i) {
    memset(&chunks[i], i, sizeof(chunks[i]));
    bool write_successful = writer.write(chunks[i]);
    if (!write_successful)
      BOOST_REQUIRE(write_successful);
  }
  writer.close();

  BufferedFileReader<types::WIB_SUPERCHUNK_STRUCT> reader("test.out", 8388608);
  types::WIB_SUPERCHUNK_STRUCT chunk;
  for (uint i = 0; i < chunks.size(); ++i) {
    bool read_successful = reader.read(chunk);
    if (!read_successful)
      BOOST_REQUIRE(read_successful);
    bool read_chunk_equals_written_chunk = !memcmp(&chunk, &chunks[i], sizeof(chunk));
    if (!read_chunk_equals_written_chunk)
      BOOST_REQUIRE(read_chunk_equals_written_chunk);
  }
  reader.close();

  remove("test.out");
}

BOOST_AUTO_TEST_SUITE_END()
