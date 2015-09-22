MRuby::Build.new do |conf|
  # load specific toolchain settings
  toolchain :gcc

  enable_debug

  conf.bins = ["mrbc"]

  conf.gem :git => 'git@github.com:AndrewBelt/mruby-yaml.git', :branch => 'master'
  conf.gem :git => 'git@github.com:mattn/mruby-base64.git', :branch => 'master'

  conf.cc do |cc|
    cc.flags = [ENV['CFLAGS'], "-I#{root}/../yaml/include"].join(" ")
  end
end
