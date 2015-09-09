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

def usual_suspects!(args)
  $solution << args.merge({
    #:foo => caller.inspect,
    #:name => "This shit came from:#{__LINE__}/#{__FILE__}",
    :sudo => true,
    :when => $toggle
  })
end

def stake_out?(*args)
  $solution << register_last_command(args, false)
  yield
end

def stake_out!(*args)
  $solution << register_last_command(args, true)
  yield
end

def investigate!(*args)
  yield
  business do
    playbook = {:hosts => "all"}.merge(*args)
    playbook[:tasks] = $solution
    YAML.dump([playbook])
  end
end
