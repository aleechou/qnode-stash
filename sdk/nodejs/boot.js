require("./api.js")
require("../common/api.run.js")
require("./window.js")
require("../common/shadow.js")

if (process.argv[1])
    require(process.argv[1])
else
    require("./")