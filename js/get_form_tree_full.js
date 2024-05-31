function nth_parent(node, n) {
    var tmp = node;
    for(var i = 0; i < n; i++) {
        if(tmp === undefined || tmp === null || tmp.parentNode === undefined || tmp.parentNode === null) return null;
        tmp = tmp.parentNode;
    }
    return tmp;
}
function get_label(node) {
    if(!node.hasAttribute("id")) {
        if(node.parentNode.tagName === "LABEL" && !node.previousSibling) return node.parentNode;
        return null;
    }
    var labels = document.querySelectorAll("label[for=\'" + node.getAttribute("id") + "\']");
    if(labels.length === 0) return null;
    for(var i = 0; i < labels.length; i++) {
        if(!document.node_visible(labels[i]) || labels[i].textContent.replace(/\s{2,}/gm, ' ').trim() === "") continue;
        return labels[i];
    }
    return null;
}
function get_text_content(node) {
    var walker = document.createTreeWalker(node, NodeFilter.SHOW_TEXT);
    var result = "";
    while(walker.nextNode()) {
        result += walker.currentNode.textContent.trim();
        result += " ";
    }
    return result;
}
var retry = false;
document.normalize();
while(true) {
    var root;

    if(document.forms.length === 0 || retry) {
        root = document.querySelector("body");
    }
    else {
        root = document.forms[0];
    }
    var pos = 0;
    var list = [];
    let args = arguments;
    var walker = document.createTreeWalker(root, NodeFilter.SHOW_ALL, {
        acceptNode(node) {
            //if(node.nodeType === Node.ELEMENT_NODE && node.tagName === "LABEL" && node.hasAttribute("for") && document.querySelector("#" + node.getAttribute("for")) !== null) return NodeFilter.FILTER_REJECT;
            if(node.tagName === "OPTION") return NodeFilter.FILTER_REJECT;
            if(node.tagName === "SCRIPT") return NodeFilter.FILTER_REJECT;
            if(!document.node_visible(node)) {
                if(node.nodeType !== Node.ELEMENT_NODE) return NodeFilter.FILTER_REJECT;
                var label = get_label(node);
                if(label === null || !document.node_visible(label)) return NodeFilter.FILTER_REJECT;
            }

            pos++;

            let tag = node.tagName;
            if(tag !== undefined) tag = tag.toUpperCase();
            var res = {};
            var skip = true;
            let class_ = (node.nodeType === Node.ELEMENT_NODE && node.hasAttribute("class")) ? node.getAttribute("class").toLowerCase() : "";

            var role = (node.nodeType === Node.ELEMENT_NODE && node.hasAttribute("role")) ? node.getAttribute("role").toLowerCase() : "";

            if(tag === "SELECT" || role == "listbox") {
                res.type = "mc";
                res.options = [];
                let options = node.querySelectorAll("option,[role=\"option\"]");
                for(var i = 0; i < options.length; i++) {
                    res.options.push({text: options[i].textContent});
                }
            }

            else if(node.nodeType === Node.TEXT_NODE) {
                res.text = node.textContent.replace(/\s{2,}/gm, ' ').trim();
                if(res.text === "") return NodeFilter.FILTER_REJECT;
                res.type = "txt";
                skip = false;
            }

            else if(tag === "TEXTAREA" || role == "textbox") {
                res.type = "ti";
            }

            else if(role == "number" || role == "range" || role == "tel") {
                res.type = "number";
                res.max = parseInt(node.getAttribute("max"));
                res.min = parseInt(node.getAttribute("min"));
                res.step = parseInt(node.getAttribute("step"));
            }

            else if(tag === "INPUT") {
                skip = false;
                let type = node.getAttribute("type").toLowerCase();
                if(type === "button" || type === "checkbox" || type === "radio") {
                    res.type = "btn";
                }
                else if(type === "text" || type === "textarea") {
                    res.type = "ti";
                }
                else if(type === "submit") {
                    res.type = "submit";
                }
                else if(type === "number" || type === "range" || type === "tel") {
                    res.type = "number";
                    res.max = parseInt(node.getAttribute("max"));
                    res.min = parseInt(node.getAttribute("min"));
                    res.step = parseInt(node.getAttribute("step"));
                }
                else if(node.hasAttribute("list")) {
                    res.type = "mc";
                    let datalist = document.querySelector("datalist[id=\"" + node.getAttribute("list") + "\"]");
                    if(datalist === null) return NodeFilter.FILTER_REJECT;

                    res.options = [];
                    let options = node.querySelectorAll("option,[role=\"option\"]");
                    for(var i = 0; i < options.length; i++) {
                        res.options.push({text: options[i].textContent, node: options[i]});
                    }
                }
                else return NodeFilter.FILTER_REJECT;
            }

            else if(tag === "BUTTON" || tag === "MAT-CHECKBOX" || tag === "MAT-RADIO-BUTTON" || (node.tagName === "A" && node.hasAttribute("href"))) {
                if(class_.includes("dropdown")) res.nogroup = true;
                if(node.hasAttribute("data-toggle")) res.nogroup = true;
                res.type = "btn";
            }

            else if(tag === "LABEL") {
                if(node.hasAttribute("for") && document.querySelector("[id=\'" + node.getAttribute("for") + "\']") !== null) return NodeFilter.FILTER_REJECT;
                if(node.childNodes.length > 0) return this.acceptNode(node.firstChild);
            }

            else if(tag === "DATALIST") {
                return NodeFilter.FILTER_REJECT;
            }

            else if(node.nodeType === Node.ELEMENT_NODE && node.tagName !== "UL") {
                if((node.onclick !== null && node.onclick !== undefined) || role === "button" || role === "radio" || role === "checkbox" || node.hasAttribute("ng-click") || class_.includes("btn") || class_.includes("button")) {

                    if(node.hasAttribute("class") && node.getAttribute("class").includes("dropdown")) res.nogroup = true;
                    if(node.hasAttribute("data-toggle")) res.nogroup = true;
                    res.type = "btn";
                }
                else return NodeFilter.FILTER_ACCEPT;
            }

            else return NodeFilter.FILTER_ACCEPT;

            if(node.nodeType === Node.ELEMENT_NODE) {
                res.node = node;

                res.label = get_label(node);
                if(res.label !== null && res.label !== undefined) res.text = res.label.textContent.replace(/\s{2,}/gm, ' ').trim();
                else res.text = get_text_content(node).replace(/\s{2,}/gm, ' ').trim();
            }

            for(var i = 0; i < args.length; i++) {
                if(args[i] !== node) continue;
                res.nogroup = true;
                break;
            }

            if(res.type !== "btn") {
                var range = document.createRange();
                range.selectNodeContents(node);
                var rects = range.getClientRects();
                if(rects.length > 0) {
                    res.rects = rects[0];
                }
            }

            res.pos = pos;
            list.push(res);
            return skip ? NodeFilter.FILTER_REJECT : NodeFilter.FILTER_ACCEPT;
        }
    });
    while(walker.nextNode()) {}

    for(var i = 0; i < list.length; i++) {
        var current = list[i];
        if(current.type !== "btn" || current.nogroup) continue;
        var group = [i];
        var prev = i;
        i++;
        var diff = -1;
        var lt = false;
        for(;i<list.length; i++) {
            let cmp = list[i];
            if(cmp.type === "txt") continue;
            if(cmp.type !== "btn" || cmp.nogroup) break;

            if(diff !== -1) {
                if(((cmp.pos)-(list[prev].pos)) !== diff) {
                    if(group.length === 2) i=i-2;
                    break;
                }

                group.push(i);
                prev = i;

                continue;
            }
            diff = (cmp.pos)-(current.pos);
            group.push(i);
            prev = i;
        }
        if(group.length > 1) {
            for(var j = 0; j < group.length; j++) {

                if(list[group[j]].lcp !== undefined) continue;
                var n = 0;
                var exit = false;
                var parent = null;
                var glength = 1;
                while(!exit) {
                    n++;
                    parent = nth_parent(list[group[j]].node, n);
                    if(parent === null) {
                        exit = true;
                        break;
                    }
                    if(parent.tagName === "BODY") break;
                    for(var k = 0; k < group.length; k++) {
                        if(k === j) continue;
                        if(list[group[k]].lcp !== undefined) continue;
                        if(parent !== nth_parent(list[group[k]].node, n)) continue;

                        var hs = nth_parent(list[group[k]].node, n-1);
                        list[group[k]].lcp = parent;
                        if((list[group[k]].text === undefined)) list[group[k]].text = hs.textContent.trim();
                        list[group[k]].pn = n;
                        list[group[k]].hs = hs;
                        glength++;

                        exit = true;
                    }
                    if(exit) {
                        var hs = nth_parent(list[group[j]].node, n-1);
                        list[group[j]].lcp = parent;
                        if((list[group[j]].text === undefined)) list[group[j]].text = hs.textContent.trim();
                        list[group[j]].pn = n;
                        list[group[j]].glength = glength;
                        list[group[j]].hs = hs;

                        var range = document.createRange();
                        range.selectNodeContents(hs);
                        var rects = range.getClientRects();
                        if(rects.length > 0) {
                            list[group[j]].rects = rects[0];
                        }
                    }
                }
            }
        }
    }
    var empty = true;
    for(var i = 0; i < list.length; i++) {
        let type = list[i].type;
        if(type === "txt") continue;
        if(type === "submit") continue;
        if(type === "btn" && list[i].glength === undefined) continue;
        empty = false;
        break;
    }
    if(!empty || retry) break;
    retry = true;
}
return list;
