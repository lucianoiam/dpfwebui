const fs = require("fs");
const loader = require("@assemblyscript/loader");
const glue = loader.instantiateSync(fs.readFileSync(__dirname + "/build/optimized.wasm"));
module.exports = glue.exports;
