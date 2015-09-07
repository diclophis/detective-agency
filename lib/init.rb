$solution = []
$toggle = nil

def register_last_command(args, if_or_unless)
  if if_or_unless
    $toggle = "last_command.rc == 0"
  else
    $toggle = "last_command.rc != 0"
  end
  { :shell => args.join(" "), :register => "last_command", :ignore_errors => true }
end

def you_need(args)
  $solution << args.merge({
    :sudo => true,
    :when => $toggle
  })
end

def unless_its_true_that?(*args)
  $solution << register_last_command(args, false)
  yield
end

def if_its_true_that?(*args)
  $solution << register_last_command(args, true)
  yield
end

def investigate!(*args)
  yield
  business do
    YAML.dump([{:hosts => "all", :tasks => $solution}])
  end
end
