FROM ubuntu:latest

RUN apt-get update -y 
RUN apt install python3 -y
RUN apt-get install python3-pip -y

WORKDIR /usr/app/

COPY  . /usr/app/

RUN pip3 install -r requirements.txt

RUN chmod +x start_api.sh
ENTRYPOINT ["/bin/bash", "start_api.sh"]