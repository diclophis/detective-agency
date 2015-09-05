# the devops detective-agency

It was a cloudy Tuesday afternoon when she walked into my office, an ubuntu-lts server from North East data centah', with ethernet cord from her hard-drives to the floor.
There was nothing but bad news in her logs, and I had a good idea how it got there...
She had a problem, her ruby was missing... and thats when I knew I had a job to do!

# how to solve the case

`detective-agency` will read a `Detectivefile`, look at the facts, and generate an ansible playbook that should solve your case.

Take for example the case of the missing ruby:

    $ cat Detectivefile
    
    # find the ruby interperter's version
    investigate! :ruby do
      unless its_true_that? "ruby -v | grep 'ruby 1'"
        you_need_to :apt => "pkg=ruby state=latest"
      end
    end

Will generate as standard output:

    $ detective-agency
    


You could then run this with `ansible` on your host:

    $ detective-agency > install-ruby.yml
    
    $ cat install-ruby.yml
    
    ---
    - hosts: all
      tasks:
      - shell: ruby -v | grep 'ruby 1'
        register: last_command
        ignore_errors: true
      - apt: pkg=ruby state=latest
        sudo: true
        when: last_command.rc != 0
    ...
    
    $ ansible-playbook -i inventory ruby.yml 

    PLAY [all] ******************************************************************** 
    
    GATHERING FACTS *************************************************************** 
    ok: [localhost]
    
    TASK: [shell ruby -v | grep 'ruby 1'] ***************************************** 
    changed: [localhost]
    
    TASK: [apt pkg=ruby state=latest] ********************************************* 
    skipping: [localhost]
    
    PLAY RECAP ******************************************************************** 
    localhost       : ok=2    changed=1    unreachable=0    failed=0
