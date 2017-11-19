require("./api.js")
require("./window.js")

if(process.argv[1])
    require(process.argv[1])
else
    require("./")
