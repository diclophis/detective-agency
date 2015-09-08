# the devops detective-agency

It was a cloudy Tuesday afternoon when she walked into my office, an ubuntu-lts server from North East data centah', with ethernet cord from her hard-drives to the floor.
There was nothing but bad news in her logs, and I had a good idea how it got there...
She had a problem, her ruby was missing... and thats when I knew I had a job to do!

# how to solve the case

`detective-agency` will read a `Detectivefile`, look at the facts, and generate an ansible playbook that should solve your case.
    
Take for example the case of the missing ruby:

```ruby
# find the ruby interperter's version
investigate! :ruby do
  if_its_true_that? "ruby -v | grep 'ruby 1.8'" do
    you_need :apt => "pkg=ruby1.8 state=absent"
  end

  unless_its_true_that? "ruby -v | grep 'ruby 1.9'" do
    you_need :apt => "pkg=ruby1.9 state=latest"
  end
end
```

You could then run `detective-agency` feeding its output to `ansible`:

    $ detective-agency | tee install-ruby.yml
    ---
    - hosts: all
      tasks:
      - shell: ruby -v | grep 'ruby 1'
        register: last_command
        ignore_errors: true
      - apt: pkg=ruby state=latest
        sudo: tru
        when: last_command.rc != 0
    ...
    
    $ ansible-playbook -i inventory install-ruby.yml 

    PLAY [all] ******************************************************************** 
    
    GATHERING FACTS *************************************************************** 
    ok: [localhost]
    
    TASK: [shell ruby -v | grep 'ruby 1'] ***************************************** 
    changed: [localhost]
    
    TASK: [apt pkg=ruby state=latest] ********************************************* 
    skipping: [localhost]
    
    PLAY RECAP ******************************************************************** 
    localhost       : ok=2    changed=1    unreachable=0    failed=0

# install from PPA

    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 14F4C975
    echo "deb http://ppa.launchpad.net/diclophis/detective-agency/ubuntu precise main" | sudo tee /etc/apt/sources.list.d/diclophis.list
    sudo apt-get update
    sudo apt-get install detective-agency
