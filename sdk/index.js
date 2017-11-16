qnode = { api: {}, window: {}, classes: {} }

// require("./window.js")

var invokeReturnCallbacks = {}
$qnode_api_invoke_return = function(reqId, value) {
    if (invokeReturnCallbacks[reqId]) {
        invokeReturnCallbacks[reqId](value)
        delete invokeReturnCallbacks[reqId]

        setTimeout(() => {}, 0);
    }
}

var connectionsSignalSlot = {}
$qnode_api_emit = function(connId) {
    if (connectionsSignalSlot[connId]) {
        connectionsSignalSlot[connId]()

        setTimeout(() => {}, 0);
    }
}

qnode.api.invoke = function(objId, methodName) {
    return new Promise((resolve) => {
        var reqId = $qnode_api_invoke.apply(null, arguments)
        invokeReturnCallbacks[reqId] = resolve
    })
}

qnode.api.on = function(objId, signalName, callback) {
    var connId = $qnode_api_on(objId, signalName)
    connectionsSignalSlot[connId] = callback
}

qnode.api.call = $qnode_api_call
qnode.api.methodList = function(objId) {
    return qnode.api.call($qnode_api_script_objects, "methodList(uint)", objId)
}
qnode.api.reflect = function(objId) {
    var meta = {
        name: qnode.api.call($qnode_api_script_objects, "className(uint)", objId),
    }
    eval("meta.methods = " + qnode.api.call($qnode_api_script_objects, "methodList(uint)", objId))
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

class Window {
    constructor(objId) {
        this.objId = objId

        if (!Window.meta) {
            Window.meta = qnode.api.reflect(objId)
        }
        if (!Window.meta.__wrapper__) {
            Window.meta.__wrapper__ = qnode.api.wrapper(Window.meta)
        }
        this.__proto__ = Window.meta.__wrapper__
    }
}

Window.meta = null

qnode.classes.Window = Window
qnode.window.create = function(url) {
    return new Promise(async(resolve) => {
        var winId = await qnode.api.invoke($qnode_api_browser_window_creator, "createBrowserWindow()")
        var window = new Window(winId)
        if (!/:\/\//.test(url))
            url = "file://" + url
        qnode.api.on(winId, "loaded(bool)", () => {
            resolve(window)
        })
        window.load(url)
    })
}