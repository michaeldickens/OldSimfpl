=begin

WARNING WARNING WARNING

This is a bad idea because it can't be undone. Instead of using this, use 
the Project Find in Xcode.

WARNING WARNING WARNING

=end

def replace filename
  file = File.new(filename, "r")
  string = file.readlines.join("")
  
#  string.gsub!(/error\("(.+)", ([01])\)/, 'value_error(\2, "\1")')
#  string.gsub!(/error_arg\("(.+)", (.+?), ([01])\)/, 'error(\3, "\1", \2)')
  string.gsub!(/[^_]error\(([01]), "(.+)"\)/, 'value_error(\1, "\2")')
  string.gsub!(/[^_]error\(([01]), (.+)", (.+?)\)/, 'value_error(\1, "\2", \3)')
  string.gsub!(/[^_]error\(([01]), (.+)\)/, 'value_error(\1, \2)')
  string.gsub!(/[^_]error\(([01]), (.+), (.+?)\)/, 'value_error(\1, \2, \3)')
#  string.gsub!(/error\((.+), ([01])\)/, 'error(\2, \1)')
#  string.gsub!(/error_arg\((.+), (.+?), ([01])\)/, 'error(\3, \1, \2)')
  file = File.new(filename, "w")
  file.write(string)
end

def replace_format filename
  file = File.new(filename, "r")
  string = file.readlines.join("")
    
  string.gsub!(/value_error\(([01]), "(.*?)%s(.*?)", value_type_to_string\((.+?)\)\)/, 'value_error(\1, "\2%t\3", \4)')
  string.gsub!(/value_error\(([01]), "(.*?)%s(.*?)", value_to_string\((.+?)\)\)/, 'value_error(\1, "\2%s\3", \4)')
  
  file = File.new(filename, "w")
  file.write(string) 
end

def replace_errors filename
  file = File.new(filename, "r")
  string = file.readlines.join("")
    
#  string.gsub!(/"Argument error: (.+?) is undefined where (.+?)"/, '"Type Error: \1 is undefined where \2"')
  string.gsub!(/"Argument error: (.+?)"/, '"Argument Error: \1"')
  string.gsub!(/"Memory error: (.+?)"/, '"Memory Error: \1"')
  string.gsub!(/"Compiler error: (.+?)"/, '"Compiler Error: \1"')
  string.gsub!(/"Domain error: (.+?)"/, '"Domain Error: \1"')
  string.gsub!(/"File error: (.+?)"/, '"IO Error: \1"')
  string.gsub!(/"IO error: (.+?)"/, '"IO Error: \1"')
  string.gsub!(/"Syntax error: (.+?)"/, '"Syntax Error: \1"')
  
  file = File.new(filename, "w")
  file.write(string)   
end

def simple filename
  file = File.new(filename, "r")
  string = file.readlines.join("")
    
  string.gsub!(/run_test_statement/, 'test_string')
  
  file = File.new(filename, "w")
  file.write(string)   
end

def type filename
  file = File.new(filename, "r")
  string = file.readlines.join("")
    
  string.gsub!(/\*(\w\S*?)\.type/, '\1.type')
  string.gsub!(/\*(\w\S*?)->type/, '\1->type')

  
  file = File.new(filename, "w")
  file.write(string)   
  
end

type("main.c")
type("eval.c")
type("interpreter.c")
type("tools.c")
type("tree.c")
type("tests.c")
type("value.c")
type("value_string.c")
type("value_regex.c")
type("value_number.c")
type("value_array.c")
type("value_list.c")
type("value_hash.c")
type("value_range.c")
type("value_block.c")
