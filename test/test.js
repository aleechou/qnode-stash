

;(async ()=>{

    var win = await qnode.window.create()

    // win.setVisible(true)
    win.load("http://www.baidu.com")

    console.log(win.qtmeta)

})()
