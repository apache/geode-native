FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y curl git gnupg2
RUN gpg2 --keyserver hkp://keys.gnupg.net --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 7D2BAF1CF37B13E2069D6956105BD0E739499BDB || \
    gpg2 --keyserver hkp://pgp.mit.edu --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 7D2BAF1CF37B13E2069D6956105BD0E739499BDB || \
    gpg2 --keyserver hkp://ha.pool.sks-keyservers.net --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 7D2BAF1CF37B13E2069D6956105BD0E739499BDB

RUN \curl -sSL https://get.rvm.io | bash
RUN /bin/bash -l -c "rvm requirements"
RUN /bin/bash -l -c "rvm install 2.3.0"
RUN /bin/bash -l -c "gem install bundler --no-ri --no-rdoc -v '=1.16.1'"

RUN /bin/bash -l -c "mkdir /temprackup"
RUN /bin/bash -l -c "echo $'source \'http://rubygems.org\' \n\
ruby \'~> 2.3.0\'\n\
gem \'rack\', \'~> 2.0.1\'\n\
gem \'rack-rewrite\'\n\
gem \'puma\' \n\
gem \'therubyracer\' \n\
gem \'sendgrid-ruby\', \'< 3.0\' \n\
gem \'elasticsearch\' ' > /temprackup/Gemfile"
RUN /bin/bash -l -c "cd /temprackup && bundle install"

RUN /bin/bash -l -c "mkdir /temp"
RUN echo 'source "https://rubygems.org"' > /temp/Gemfile
RUN echo "gem 'bookbindery'" >> /temp/Gemfile
RUN echo "gem 'libv8', '3.16.14.7'" >> /temp/Gemfile
RUN /bin/bash -l -c "cd /temp && bundle install"

RUN /bin/bash -l -c "cp /etc/hosts ~/hosts.new"
RUN /bin/bash -l -c 'sed -i -E "s/(::1\s)localhost/\1/g" ~/hosts.new'

RUN echo "alias rackup='rackup -o 0.0.0'" >> /etc/profile

EXPOSE 9292

ENTRYPOINT ["/bin/sh", "-c" , "cat ~/hosts.new > /etc/hosts && . /etc/profile && alias rackup='rackup -o 0.0.0.0' && /bin/bash -l" ]