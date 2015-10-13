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

def wtf
return <<EOJS
var Rutty = function(argv, terminal) {
  this.terminalForm = terminal;
  this.socket = terminal.dataset.socket;
  this.consoleUid = terminal.dataset.console;
  this.argv_ = argv;
  this.io = null;
  this.sendingSweep = null;
  this.pendingString = [];
  this.sendingResize = null;
  this.pendingResize = [];
  this.source = new EventSource(this.terminalForm.action + '?socket=' + this.socket); //TODO: fix socket params passing, put in session somehow
  this.source.onopen = function(event) {
    this.io.terminal_.reset(); //TODO: resume, needs to keep some buffer
  }.bind(this);
  this.source.onmessage = function(event) {
    var msg = JSON.parse(event.data);

    if (msg.raw && msg.raw.length > 0) { // see: https://github.com/flori/json for discussion on `to_json_raw_object`
      var decoded = '';
      for (var i=0; i<msg.raw.length; i++) {
        //NOTE: what is the difference here?
        decoded += String.fromCodePoint(msg.raw[i]); // & 0xff ??
        //decoded += String.fromCharCode(msg.raw[i]); // & 0xff ??
      }

      this.io.writeUTF16(decoded);
    }
  }.bind(this);
  this.source.onerror = function(e) {
    this.source.close();
    this.io.writeUTF16('Stream close...');
    this.connected = false;
  }.bind(this);
  this.connected = true;
};

Rutty.prototype.run = function() {
  this.io = this.argv_.io.push();

  this.io.onVTKeystroke = this.sendString_.bind(this);
  this.io.sendString = this.sendString_.bind(this);
  this.io.onTerminalResize = this.onTerminalResize.bind(this);
};

Rutty.prototype.sendString_ = function(str) {
  if (!this.connected || !this.consoleUid) {
    return;
  }

  if (this.sendingSweep === null) {
    this.sendingSweep = true;

    var oReq = new XMLHttpRequest();
    oReq.onload = function(e) {
      this.sendingSweep = null;
      if (this.pendingString.length > 0) {
        var joinedPendingStdin = this.pendingString.join('');
        this.pendingString = [];
        this.sendString_(joinedPendingStdin);
      }
    }.bind(this);
    oReq.onerror = function(e) {
      this.source.close();
      this.connected = false;
    }.bind(this);

    var formData = new FormData();
    var in_d = JSON.stringify({data: str});
    formData.append('in', in_d);
    formData.append('socket', this.socket);
    oReq.open('POST', this.terminalForm.action + '/stdin', true);
    oReq.send(formData);
  } else {
    this.pendingString.push(str);
  }
};

Rutty.prototype.onTerminalResize = function(cols, rows) {
  if (!this.connected || !this.consoleUid) {
    return;
  }

  if (this.sendingResize) {
    //clearTimeout(this.sendingResize);
    //this.sendingResize.abort();
    this.pendingResize.push([cols, rows]);
  } else {
    var oReq = new XMLHttpRequest();
    oReq.onload = function(e) {
      this.sendingResize = null;
      if (this.pendingResize.length > 0) {
        var lastResize = this.pendingResize.pop();
        this.pendingResize = [];
        this.onTerminalResize(lastResize[0], lastResize[1]); // only send the latest pendingResize
      }
    }.bind(this);

    var formData = new FormData();
    formData.append('rows', rows);
    formData.append('cols', cols);
    formData.append('socket', this.socket); //TODO: figure out socket params passing, use session
    oReq.open('POST', this.terminalForm.action + '/resize', true);
    oReq.send(formData);

    this.sendingResize = oReq;
  }
};

var initTerminal = function(terminalElement) {
  if (terminalElement.dataset.status != 'close') {
    var child = null;
    while(terminalElement.firstChild) {
      child = terminalElement.removeChild(terminalElement.firstChild);
    }

    var term = null;
    var ws = null;

    lib.init(function() {

      term = new hterm.Terminal();
      term.decorate(terminalElement);

      term.setWidth(20);
      term.setHeight(10);

      term.setCursorPosition(0, 0);
      term.setCursorVisible(true);
      term.prefs_.set('ctrl-c-copy', true);
      term.prefs_.set('use-default-window-copy', true);

      var aRutty = function(argv) {
        return new Rutty(argv, terminalElement);
      };

      term.runCommandClass(aRutty);

      term.command.onTerminalResize(
        term.screenSize.width,
        term.screenSize.height
      );
    });
  }
};

window.addEventListener('load', function(documentLoadedEvent) {
  var terminal = document.getElementById("terminal");
  initTerminal(terminal);
  console.log('foo');
});
EOJS
end

def index
  "<html>" +
    "<head>" +
      "<link rel='icon' href='data:;base64,iVBORw0KGgo='>" +
      "<style>html, body { background: black; margin: 0; padding: 0; }</style>" +
      "<script src='hterm'></script>" +
    "</head>" +
    "<body>" +
      "<div id='terminal' data-socket='' data-console=''></div>" +
      "<script>#{wtf}</script>" +
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





