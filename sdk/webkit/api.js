qnode = {
    api: {}
}

qnode.api.runScriptInThread = function(threadObjId, script) {
    return $window.runScriptInThread(threadObjId, script)
}