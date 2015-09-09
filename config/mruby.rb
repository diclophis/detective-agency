MRuby::Build.new do |conf|
  # load specific toolchain settings
  toolchain :gcc

  enable_debug

  conf.bins = ["mrbc"]
end
