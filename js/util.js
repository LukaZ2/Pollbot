document.node_rendered = function (elem) {
    if (!(elem instanceof Element)) return true;
    const style = getComputedStyle(elem);
    if (style.display === 'none') return false;
    if (style.visibility !== 'visible') return false;
    if (style.opacity < 0.1) return false;
    if (elem.offsetWidth + elem.offsetHeight + elem.getBoundingClientRect().height +
        elem.getBoundingClientRect().width === 0) {
        return false;
    }
    const elemCenter   = {
        x: elem.getBoundingClientRect().left + elem.offsetWidth / 2,
        y: elem.getBoundingClientRect().top + elem.offsetHeight / 2
    };
    if (elemCenter.x < 0) return false;
    if (elemCenter.x > (document.documentElement.clientWidth || window.innerWidth)) return false;
    if (elemCenter.y < 0) return false;
    if (elemCenter.y > (document.documentElement.clientHeight || window.innerHeight)) return false;
    let pointContainer = document.elementFromPoint(elemCenter.x, elemCenter.y);
    do {
        if (pointContainer === elem) return true;
    } while (pointContainer = pointContainer.parentNode);
    return false;
}
document.node_visible = function (el) {
    if(el.nodeType !== Node.ELEMENT_NODE) return el.parentNode !== null && el.parentNode !== undefined ? document.node_visible(el.parentNode) : true;
    if(!el.checkVisibility()) return false;
    var style = window.getComputedStyle(el);
    //if(!document.node_rendered(el)) return false;
    return style === null || style === undefined || (style.visibility !== "hidden");
}
