# always yield to investigation
def investigate!(*args)
  yield
  business do
    tasks
  end
end

# if suspect surveillance indicates false
def stake_out?(*args)
  if register_last_command(args, false)
    yield
  end
end

def stake_out!(*args)
  if register_last_command(args, true)
    yield
  end
end
