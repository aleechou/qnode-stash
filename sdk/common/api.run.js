var run_invoke_id = 0
var run_callbacks = {}
qnode.api.run = function(objId, func, passedArgvs) {
    var invoke_id = run_invoke_id++,
        argvs = [],
        values = []
    if (passedArgvs) {
        for (var argvName in passedArgvs) {
            argvs.push(argvName)
            switch (typeof passedArgvs[argvName]) {
                case 'function':
                    values.push(passedArgvs[argvName].toString())
                    break
                case 'undefined':
                    values.push('undefined')
                    break
                default:
                    values.push(JSON.stringify(passedArgvs[argvName]))
                    break
            }
        }
    }
    var strfunc = `
var from = ${qnode.api.threadId}
var resolve = function(val){
    qnode.api.run(from, ()=>{
        qnode.api.onRunResovle( ${invoke_id}, val )
    }, {val})
}
;(function(${argvs.join(',')}){
try{
(${func.toString()})()
}catch(e){
    console.error(e)
}
})( ${values.join(',')} )`

    qnode.api.runScriptInThread(objId, strfunc)

    return new Promise((resolve) => {
        run_callbacks[invoke_id] = resolve
    })
}

qnode.api.onRunResovle = function(invokeId, val) {
    if (run_callbacks[invokeId]) {
        run_callbacks[invokeId](val)
        delete run_callbacks[invokeId]
    }
    setImmediate(() => {})
}