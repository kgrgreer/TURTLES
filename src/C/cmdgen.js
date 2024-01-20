// cmdgen.js - generates cmds.h and cmds.c

const fs = require('node:fs');

var fnDefs = '', scopeDefs = '', declDefs = '', cmdToStr = '';

function sf(args, code) {
  var argDefs = args.split(',').reverse().map(a => `  long ${a} = (long) pop(stack);`).join('\n');
  var body    = code ? `  push(stack, (void*) (long) (${code}));` : '';

  return argDefs + '\n' + body;
}

function af(args, code) {
  var argDefs = args.split(',').reverse().map(a => `  long ${a} = (long) pop(stack);`).join('\n');

  return argDefs + '\n' + code;
}

global.sf = sf;
global.af = af;

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

  var aa = args.split(',');

  var argDef = aa.map(arg => {
    var [type, name] = arg.split(' ');
    return `  ${type} ${name} = (${type}) nextI();`;
  }).join('\n');

fnDefs += `void ${name}() {
${argDef}
${code};
}

`;

  declDefs += `void ${name}();\n`;
  cmdToStr += `  if ( fn == &${name} ) return "${name}";\n`;

  if ( emit ) {
    // TODO: complete

    const argCode1 = aa.map(arg => {
      var [type, name] = arg.split(' ');
      return `  void* ${name} = nextI();`;
    }).join('\n');
    const argCode2 = aa.map(arg => {
      var [type, name] = arg.split(' ');
      return `  push(code, ${name});`;
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
#include <sys/time.h>
#include "t0.h"
#include "globals.h"

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
