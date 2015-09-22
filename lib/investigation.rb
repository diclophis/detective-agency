# basic implementation of DSL

=begin
def tasks
  YAML.dump([{:name => "stuff"}])
end

def push_command(args)
  return true
end

def register_last_command(args, if_or_unless)
  return true
end
=end

def usual_suspects!(args)
  push_command(args)
end

def tasks(args)
  playbook = {:hosts => "all"}.merge(*args)
  playbook[:tasks] = @tasks
  YAML.dump([playbook])
end

def push_command(args)
  @tasks << args.merge({
    :sudo => true,
    :when => @toggle
  })
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

def index
  "<html>" +
    "<head>" +
      "<link rel='icon' href='data:;base64,iVBORw0KGgo='>" +
      "<style>body { background: blue; }</style>" +
      "<script src='hterm'></script>" +
    "</head>" +
    "<body>" +
      "<h1>test</h1>" +
    "</body>" +
  "</html>"
end

def stack_byte(b)
  if b.length == 2
    @bytes ||= String.new
    @bytes += b[0]

    return @bytes[-2, 2] == "\r\n"
  end
end

def response_for_token(token)
  r = @bytes
  b = "GET /#{token} HTTP"
  if @bytes.index(b) == 0
    case token
      when ""
        r = index
        c = "text/html"
      when "hterm"
        r = hterm
        c = "text/javascript"
    end
  end

  @bytes = nil

  return "HTTP/1.1 200 OK\r\nContent-Type: #{c}\r\nContent-Length: #{r.length}\r\n\r\n#{r}"
end





