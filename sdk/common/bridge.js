var remote_objects_by_id = {}

var local_objects_by_id = {}
var local_objects_signned_id = 0


qnode.bridge.object = function(object, bridgeFrom, remoteId) {

    var meta = {
        bridgeFrom: bridgeFrom,
        remoteId: remoteId,
        id: local_objects_signned_id++,
        bridgeTo: [],
        isOrigin: bridgeFrom == undefined,
        isShadow: bridgeFrom != undefined,
    }

    if (meta.isShadow) {
        meta.bridgeTo.push([bridgeFrom, remoteId])
    }

    var localbject = proxy(object, meta, [])
    local_objects_by_id[meta.id] = localbject

    return localbject
}

qnode.bridge.bridgeTo = async function(threadId, object, funcReceiverInWindow) {
    if (!funcReceiverInWindow)
        funcReceiverInWindow = () => {}

    var remoteId = await qnode.api.run(threadId, () => {

        shadowObject = qnode.bridge.object(object, from, originId)

        resolve(shadowObject.meta.id)
        console.log(">>>", shadowObject.meta.id)

        funcReceiverInWindow(object)

    }, { originId: object.meta.id, object, funcReceiverInWindow })

    console.log("remoteId:", remoteId)

    object.meta.bridgeTo.push([threadId, remoteId])

    console.log(object.meta)

    return remoteId
}


function proxy(target, meta, path) {

    // var localObject = local_objects_by_id[id]

    for (var name in target) {
        if ("object" == typeof target[name] && target[name].constructor != Array) {
            target[name] = proxy(target[name], meta, path.concat([name]))
        }
    }

    var callbackid_signned = 0
    var changeCallbacks = {}
    var onchange = (prop, callback) => {
        if (!changeCallbacks[prop])
            changeCallbacks[prop] = {}
        var callbackid = callbackid_signned++;
        changeCallbacks[prop][callbackid] = callback
        return () => delete changeCallbacks[prop][callbackid]
    }

    var proxyobj = new Proxy(target, {
        set: function(target, prop, value) {

            // 没有变化
            if (target[prop] === value) {
                return true
            }

            if ("object" == typeof value && value.constructor != Array) {
                target[prop] = proxy(value, meta, path.concat([name]))
            } else {
                target[prop] = value
            }

            // 同步桥接的对象
            for (var remote of meta.bridgeTo) {
                var threadId = remote[0]
                var remoteId = remote[1]

                // 变化来源
                if ($mutation_from == threadId && $mutation_for == remoteId) {
                    continue
                }

                qnode.api.run(threadId, () => {
                    qnode.bridge.commit(remoteId, path, prop, value, from)
                }, {
                    remoteId,
                    path,
                    prop,
                    value,
                    from: qnode.api.threadId
                })
            }

            // 触发事件
            if (changeCallbacks[prop]) {
                for (var id in changeCallbacks[prop]) {
                    setImmediate(changeCallbacks[prop][id], value, prop, path, local_objects_by_id[meta.id])
                }
            }

            return true
        },
        get: function(target, prop) {
            if (prop == "meta") {
                return meta
            } else if (prop == "change") {
                return onchange
            }

            // 这个函数 es6 尚未实现
            else if (prop == Symbol.toStringTag) {
                return function() {}
            }

            return (meta && meta.hasOwnProperty(prop)) ? meta[prop] : target[prop]
        },
    })
    return proxyobj
}

var $mutation_from = undefined
var $mutation_for = undefined

qnode.bridge.commit = function(localId, path, prop, value, from) {
    var localObject = local_objects_by_id[localId]
    if (!localObject) {
        console.error("unknow local object id for qnode.bridge.commit() ", localId)
        return
    }

    $mutation_from = from
    $mutation_for = localId

    path.reduce((object, name) => object[name], localObject)[prop] = value

    $mutation_from = undefined
    $mutation_for = undefined
}