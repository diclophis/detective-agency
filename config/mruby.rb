MRuby::Build.new do |conf|
  # load specific toolchain settings

  # Gets set by the VS command prompts.
  if ENV['VisualStudioVersion'] || ENV['VSINSTALLDIR']
    toolchain :visualcpp
  else
    toolchain :gcc
  end

  #enable_debug

  # Use mrbgems
  conf.gem :git => 'git@github.com:AndrewBelt/mruby-yaml.git', :branch => 'master'
  # conf.gem :git => 'git@github.com:zzak/mruby-optparse.git', :branch => 'master'
  # conf.gem :git => 'git@github.com:mattn/mruby-onig-regexp.git', :branch => 'master'

  # include the default GEMs
  # conf.gembox 'default'
end

MRuby::Build.new('host-debug') do |conf|
  # load specific toolchain settings

  # Gets set by the VS command prompts.
  if ENV['VisualStudioVersion'] || ENV['VSINSTALLDIR']
    toolchain :visualcpp
  else
    toolchain :gcc
  end

  enable_debug

  # include the default GEMs
  conf.gembox 'default'

  # C compiler settings
  conf.cc.defines = %w(ENABLE_DEBUG)

  # Generate mruby debugger command (require mruby-eval)
  conf.gem :core => "mruby-bin-debugger"
end
