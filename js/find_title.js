function adjacent_text(node) {
    var result = "";
    while(node) {
        if(node.nodeType !== Node.TEXT_NODE && (node.childNodes.length !== 1 || node.firstChild.nodeType !== Node.TEXT_NODE)) break;
        result+=node.textContent;
        node = node.nextSibling;
    }
    return result;
}
var result = [];
document.normalize();
for(var i = 0; i < arguments.length; i++) {
    var walker = document.createTreeWalker(document.querySelector("body"), NodeFilter.SHOW_TEXT);
    var best = {"ts": null};

    while(walker.nextNode()) {
        var node = walker.currentNode;
        if(node.parentElement.nodeName === "LABEL" && node.parentElement.hasAttribute("for") && document.querySelector("input[id=\'" + node.parentElement.getAttribute("for") + "\']") !== null) continue;
        var style = window.getComputedStyle(node.parentElement);
        if(style != null && style.visibility == "hidden") continue;
        if(style != null && style.display == "none") continue;
        if(node.textContent.replace(/\s{2,}/gm, '').trim() == "") continue;

        //if(!node.textContent.includes(" ")) continue;
        var range = document.createRange();
        range.selectNodeContents(node);
        var rects = range.getClientRects();
        if(rects.length === 0) continue;
        if(rects[0].y + rects[0].height >= arguments[i].y) continue;

        var xd = (arguments[i].x)-(rects[0].x);
        var yd = (arguments[i].y)-(rects[0].y);
        if(xd < 0) xd = xd*(-1);
        if(yd < 0) yd = yd*(-1);

        var size = parseInt(style.fontSize);
        if(size === null) size = 1;
        var ts = ((xd*4)+(yd))-(size*30);
        if(style.fontStyle === "italic") ts+=400;

        if(best.ts === null || best.ts > ts) best = {"ts": ts, "text": adjacent_text(node)};
    }
    result.push(best.text.trim());
}
return result;
