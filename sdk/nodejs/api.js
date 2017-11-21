qnode = {
    api: {
        threadId: $qnodeapi_thread
    },
    bridge: {},
    window: {},
    classes: {}
}

var invokeReturnCallbacks = {}
$qnodeapi_invoke_return = function(reqId, value) {
    if (invokeReturnCallbacks[reqId]) {
        invokeReturnCallbacks[reqId](value)
        delete invokeReturnCallbacks[reqId]

        setTimeout(() => {}, 0);
    }
}

var connectionsSignalSlot = {}
$qnodeapi_emit = function(connId) {
    if (connectionsSignalSlot[connId]) {
        connectionsSignalSlot[connId]()

        setTimeout(() => {}, 0);
    }
}

qnode.api.invoke = function(objId, methodName) {
    return new Promise((resolve) => {
        var reqId = $qnodeapi_invoke.apply(null, arguments)
        invokeReturnCallbacks[reqId] = resolve
    })
}

qnode.api.on = function(objId, signalName, callback) {
    var connId = $qnodeapi_on(objId, signalName)
    connectionsSignalSlot[connId] = callback
}

qnode.api.call = $qnodeapi_call
qnode.api.methodList = function(objId) {
    return qnode.api.call($qnodeapi_script_objects, "methodList(uint)", objId)
}
qnode.api.reflect = function(objId) {
    var meta = {
        name: qnode.api.call($qnodeapi_script_objects, "className(uint)", objId),
    }
    eval("meta.methods = " + qnode.api.call($qnodeapi_script_objects, "methodList(uint)", objId))
    return meta
}
qnode.api.wrapper = function(qtClassMeta) {
    var wrapper = {
        qtmeta: qtClassMeta
    }
    for (var metaMethod of qtClassMeta.methods) {
        wrapper[metaMethod.name] = ((signature) => {
            return function() {
                arguments.__proto__ = Array.prototype
                arguments.unshift(signature)
                arguments.unshift(this.objId)
                return qnode.api.invoke.apply(this, arguments)
            }
        })(metaMethod.signature)
    }
    return wrapper
}

qnode.api.runScriptInThread = function(threadObjId, script) {
    return qnode.api.invoke(threadObjId, "runScript(QString)", script)
}