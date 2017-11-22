const EventEmitter = require('events');

class Window extends EventEmitter {
    constructor(objId) {
        super()
        this.objId = undefined
        this._on_qt_ready = undefined
    }

    on(event, callback) {
        if (event == "ready") {
            this._hook_qt_ready()
        }
        return EventEmitter.prototype.on.apply(this, arguments)
    }
    once(event, callback) {
        if (event == "ready") {
            this._hook_qt_ready()
        }
        return EventEmitter.prototype.once.apply(this, arguments)
    }

    _hook_qt_ready() {
        if (!this._on_qt_ready) {
            this._on_qt_ready = (ok) => {
                this.emit('ready', ok)
            }
            qnode.api.on(this.objId, "ready(bool)", this._on_qt_ready)
        }
    }

    _oncreated(objId) {
        if (!Window.meta) {
            Window.meta = qnode.api.reflect(objId)
            Window.meta.__wrapper__ = qnode.api.wrapper(Window.meta)
            for (var name in Window.meta.__wrapper__) {
                if (!Window.prototype[name])
                    Window.prototype[name] = Window.meta.__wrapper__[name]
            }
        }
        this.objId = objId
    }

    load(url) {
        return new Promise((resolve) => {
            qnode.api.invoke(this.objId, "load(QString)", url)
            this.once("ready", (ok) => {
                resolve(ok)
            })
        })
    }

    run(func, vars) {
        return qnode.api.run(this.objId, func, vars)
    }

    bridgeShadowObject(object, funcReceiverInWindow) {
        qnode.bridge.bridgeTo(this.objId, object, funcReceiverInWindow)
    }
}

objectById = {}

qnode.window.eventEmit = function(objId, eventName, argv) {
    if (!objectById[objId]) {
        return
    }
    objectById[objId].emit(eventName, ...argv)
}

qnode.classes.Window = Window
qnode.window.create = async function() {
    return new Promise((resolve) => {
        var window = new Window()
        qnode.api.invoke($qnodeapi_browser_window_creator, "createBrowserWindow(uint)", qnode.api.threadId)
            .then((objId) => {
                objectById[objId] = window
                window._oncreated(objId)
                resolve(window)
            })
        return window
    })
}
qnode.window.openConsole = async function() {
    var inspector = await qnode.window.create()
    inspector.load("http://127.0.0.1:" + $qnodeapi_console_port)
    inspector.show()
}
