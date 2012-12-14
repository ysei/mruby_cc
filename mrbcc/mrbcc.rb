require_relative './rite_parser'
# encoding: utf-8
require_relative './preparser'
require_relative './codegen'
require_relative './mrb_opcodes'
require 'fileutils'

MRUBY_PATH = File.expand_path("../../mruby", __FILE__)
MRBC_BIN = "#{MRUBY_PATH}/bin/mrbc"
TMP_DIR = File.expand_path("../../tmp", __FILE__)

it = ARGV[0]
RB_FILE_PATH = if it.start_with?("/") || it[1] == ":"
  it
else
  "#{Dir.pwd}/#{it}"
end
RB_FILE_DIR = File.expand_path("..", RB_FILE_PATH)
BUILD_DIR = File.expand_path("../../build", __FILE__)

puts "Compiling..."

opcodes = MrbOpcodes.new(MRUBY_PATH)

# preparse file so we can #include
rb_filename = RB_FILE_PATH
rb_filename_noext = File.basename(rb_filename, ".rb")
Preparser.preparse(rb_filename, "#{TMP_DIR}/tmp_out.rb")

# compile to .mrb
str = %x[cd "#{TMP_DIR}" && #{MRBC_BIN} tmp_out.rb]

# parse
parser = RiteParser.new("#{TMP_DIR}/tmp_out.mrb")

# create C file
File.open("#{BUILD_DIR}/c_files/out.c", "w") do |wf|
  wf.write(OpcodeParser.new(parser, opcodes.opcodes, "script_entry_point", 0).process_irep)
end

# compile C file
puts %x[cd "#{BUILD_DIR}" && make 2>&1 | grep "error:"]

# copy C file
FileUtils.mv("#{BUILD_DIR}/mrbcc_out.so", "#{RB_FILE_DIR}/#{rb_filename_noext}.so")

# clean up
FileUtils.rm(rb_filename.gsub(/\.rb$/, ".mrb"), :force => true)
FileUtils.rm(File.expand_path("#{TMP_DIR}/tmp_out.mrb", __FILE__), :force => true)
FileUtils.rm(File.expand_path("#{TMP_DIR}/tmp_out.rb", __FILE__), :force => true)
