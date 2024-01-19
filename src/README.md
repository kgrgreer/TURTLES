# C
To run the C version of T0, do:
`$ cd C; ./build.sh; ./a.out`

or:

`$ cd C; ./build.sh; ./a.out < test.t0`


# JS
To run the JS version of T0:

 1. run an HTTP server, something like: `$ python -m SimpleHTTPServer 8082` or `$ http-server`
 2. point your browser to [http://localhost:8082/](http://localhost:8082/)
 3. open the browser console

The JS version was really just meant to be a prototype for the C and assembly versions,
but if it is going to continue to be used, then nodejs support should be added.

You can install http-server with `$ npm install http-server -g`.
