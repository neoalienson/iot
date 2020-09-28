FROM node:8
RUN npm install -g coffeescript
RUN mkdir /codecombat
COPY source/ /codecombat
EXPOSE 3000
WORKDIR /codecombat
CMD [ "coffee", "multicore.coffee"]
