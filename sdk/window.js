
const EventEmitter = require('events');

class Window extends EventEmitter {
    constructor(objId) {
        super()
        this.objId = undefined
        this._on_qt_ready = undefined
    }

    on(event, callback) {
        if(event=="ready") {
            this._hook_qt_ready()
        }
        return EventEmitter.prototype.on.apply(this, arguments)
    }
    once(event, callback) {
        if(event=="ready") {
            this._hook_qt_ready()
        }
        return EventEmitter.prototype.once.apply(this, arguments)
    }

    _hook_qt_ready() {
        if(!this._on_qt_ready) {
            this._on_qt_ready = (ok)=>{
                console.log("readyreadyready")
                this.emit('ready', ok)
            }
            qnode.api.on(this.objId, "ready(bool)", this._on_qt_ready)
        }
    }

    _oncreated(objId) {
        if(!Window.meta) {
            Window.meta = qnode.api.reflect(objId)
            Window.meta.__wrapper__ = qnode.api.wrapper(Window.meta)
            for(var name in Window.meta.__wrapper__) {
                Window.prototype[name] = Window.meta.__wrapper__[name]
            }
        }
        this.objId = objId
    }

    load (url) {
        return new Promise((resolve)=>{
            qnode.api.invoke(this.objId, "load(qstring)", url)
            this.once("ready",(ok)=>{
                console.log("okkkkkkkk")
                resolve(ok)
            })
        })
    }
}

qnode.classes.Window = Window
qnode.window.create = async function() {
    return new Promise((resolve)=>{
       var window = new Window()
       qnode.api.invoke($qnodeapi_browser_window_creator, "createBrowserWindow(uint)", $qnodeapi_thread)
           .then((objId)=>{
                window._oncreated(objId)
                resolve(window)
             })
       return window
    })
}
