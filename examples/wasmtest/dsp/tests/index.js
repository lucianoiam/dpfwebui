const assert = require("assert");
const myModule = require("..");
assert.notEqual(Object.keys(myModule).indexOf('getLabel'), -1);
console.log("ok");
