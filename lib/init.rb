$solution = []

def you_need_to(args)
  args.merge({
    :sudo => true,
    :when => "last_command.rc != 0",
  })
end

def its_true_that?(*args)
  $solution << { :shell => args.join(" "), :register => "last_command", :ignore_errors => true }
  return false
end

def investigate!(*args)
  $solution << yield
  business do
    YAML.dump([{:hosts => "all", :tasks => $solution}])
  end
end
