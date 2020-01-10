FROM debian:10
LABEL maintainer "nymea GmbH <developer@nymea.io>"

# Environment
ENV DEBIAN_FRONTEND noninteractive
ENV LANGUAGE en_US

# Add repository
RUN apt-get update
RUN apt-get install -y gnupg apt-utils
RUN echo 'deb http://repository.nymea.io buster main non-free' > /etc/apt/sources.list.d/nymea.list
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-key A1A19ED6

# Install packages
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y nymea nymea-plugins nymea-cli

CMD ["/usr/bin/nymead", "-n"]
