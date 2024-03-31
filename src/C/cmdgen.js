// cmdgen.js - generates cmds.h and cmds.c

const fs = require('fs');

var fnDefs = '', scopeDefs = '', declDefs = '', cmdToStr = '';

function comma(s) {
  return s === '' ? [] : s.split(',');
}

// Argument Function - defines local variables with values taken from stack
function af(args, code) {
  var argDefs = comma(args).reverse().map(a => `  long ${a} = (long) pop(stack);`).join('\n');
  return argDefs + '\n' + code;
}

// Stack Function - like af(), put pushes result back onto the Stack
function sf(args, code) {
  return af(args, code ? `  push(stack, (void*) (long) (${code}));` : '');
}

global.af = af;
global.sf = sf;

const { CMDS, INSTRUCTIONS } = require('./cmds.js');


CMDS.forEach(i => {
  var [ name, op, code ] = i;
  fnDefs += `
void ${name}() {
${code}
}
`;

  scopeDefs += `  scope = addFn(scope, "${op}", &${name});\n`;

  declDefs += `void ${name}();\n`;

  cmdToStr += `  if ( fn == &${name} ) return "${op}";\n`;
});


INSTRUCTIONS.forEach(i => {
  var [ name, args, code, emit ] = i;

  declDefs += `void ${name}();\n`;
  cmdToStr += `  if ( fn == &${name} ) return "${name}";\n`;

  args = comma(args);

  var argDef = args.map(arg => {
    var [type, name] = arg.split(' ');
    return `  ${type} ${name} = (${type}) nextI();`;
  }).join('\n');

fnDefs += `void ${name}() {
${argDef}
${code};
}

`;

  if ( emit ) {
    const argCode1 = args.map(arg => {
      var [type, name] = arg.split(' ');
      return `  ${type} ${name} = (${type}) nextI();`;
    }).join('\n');
    const argCode2 = typeof emit === 'string' ? emit + ';' : args.map(arg => {
      var [type, name] = arg.split(' ');
      return `  push(code, (void*) ${name});`;
    }).join('\n');

    const signature = `void emit${name[0].toUpperCase() + name.substring(1)}()`;
    fnDefs += `${signature} {
${argCode1}
  push(code, ${name});
${argCode2}
}

`;

    declDefs += signature + ';\n';
  }
});


fs.writeFileSync('cmds.h', `// cmds.h -- generated by cmdgen.js

Scope* addCmds(Scope* scope);

char* cmdToStr(Fn fn);

${declDefs}
`);


fs.writeFileSync('cmds.c', `// cmds.c -- generated by cmdgen.js

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include "t0.h"
#include "globals.h"
#include "cmds.h"

${fnDefs}

Scope* addCmds(Scope* scope) {
${scopeDefs}
  return scope;
}

char* cmdToStr(Fn fn) {
${cmdToStr}

  return NULL;
}
`);
