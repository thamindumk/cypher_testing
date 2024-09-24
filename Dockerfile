FROM aaa:latest

WORKDIR /home/ubuntu
RUN mkdir antlr
WORKDIR /home/ubuntu/antlr
RUN apt-get update && apt-get install --no-install-recommends -y default-jre
RUN curl -O https://s3.amazonaws.com/artifacts.opencypher.org/M23/Cypher.g4
RUN curl -O https://www.antlr.org/download/antlr-4.13.2-complete.jar
RUN java -jar antlr-4.13.2-complete.jar -Dlanguage=Cpp -visitor Cypher.g4
RUN apt-get purge default-jre -y

WORKDIR /home/ubuntu
RUN mkdir cypher
WORKDIR /home/ubuntu/cypher
RUN mkdir generated && cd generated
RUN mv /home/ubuntu/antlr/*.cpp /home/ubuntu/cypher/generated/
RUN mv /home/ubuntu/antlr/*.h /home/ubuntu/cypher/generated/
RUN rm -f /home/ubuntu/antlr/*

WORKDIR /home/ubuntu/cypher
RUN mkdir ast_generator
COPY /ast_generator/*.cpp /home/ubuntu/cypher/ast_generator/
COPY /ast_generator/*.h /home/ubuntu/cypher/ast_generator/
COPY main.cpp /home/ubuntu/cypher/
COPY CMakeLists.txt /home/ubuntu/cypher/

RUN mkdir /home/ubuntu/cypher/build
WORKDIR /home/ubuntu/cypher/build

RUN cmake ..
RUN make
RUN ./cypher


