require("./api.js")
require("../common/api.run.js")
require("./window.js")
require("../common/bridge.js")

if (process.argv[1])
    require(process.argv[1])
else
    require("./")
