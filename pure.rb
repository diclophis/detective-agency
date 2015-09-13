#!/usr/bin/env ruby

require 'yaml'

require './lib/detective.rb'
require './lib/investigation.rb'

def business(*args)
  puts yield
end

eval(File.readlines('./Detectivefile').join)
