# basic implementation of DSL

def tasks
  "---\n- name: stuff"
end

def push_command(args)
  return true
end

def register_last_command(args, if_or_unless)
  return true
end

def usual_suspects!(args)
  push_command(args)
end

=begin
def push_command(args)
  @tasks << args.merge({
    :sudo => true,
    :when => @toggle
  })
end

def tasks
  playbook = {:hosts => "all"}.merge(*args)
  playbook[:tasks] = @tasks
  #YAML.dump([playbook])
end

def register_last_command(args, if_or_unless)
  @tasks ||= []
  @toggle ||= nil
  if if_or_unless
    @toggle = "last_command.rc == 0"
  else
    @toggle = "last_command.rc != 0"
  end
  @tasks << { :shell => args.join(" "), :register => "last_command", :ignore_errors => true }
end
=end
