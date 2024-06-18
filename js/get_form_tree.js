function node_visible(node) {
    if(node === null) return false;
    if(node.nodeType !== Node.ELEMENT_NODE) return node.parentNode !== null && node.parentNode !== undefined ? node_visible(node.parentNode) : true;
    if(!node.checkVisibility({contentVisibilityAuto: true, visibilityProperty: true, opacityProperty: true})) return false;
    var style = window.getComputedStyle(node);
    //if(!node.checkVisibility({visibilityProperty: true})) return false;
    if(style.display === "none") return false;
    //if(!node.checkVisibility({opacityProperty: true})) return false;
    var rect = node.getBoundingClientRect();
    if((node.offsetWidth + node.offsetHeight + rect.width + rect.height) === 0) return false;
    if(rect.left < 0) return false;
    if(rect.top < 0) return false;
    return true;
}
function nth_parent(node, n) {
    var tmp = node;
    for(var i = 0; i < n; i++) {
        if(tmp === undefined || tmp === null || tmp.parentNode === undefined || tmp.parentNode === null) return null;
        tmp = tmp.parentNode;
    }
    return tmp;
}
const label_tags = ["INPUT", "METER", "PROGRESS", "SELECT", "TEXTAREA"];
function get_first_non_empty(node, for_id) {
    for(var i = 0; i < node.childNodes.length; i++) {
        let tmp = node.childNodes[i];
        if(tmp.nodeType !== Node.ELEMENT_NODE) continue;
        //if(!node_visible(tmp)) continue;
        if(!label_tags.includes(tmp.tagName)) continue;
        if(for_id === undefined || for_id === null) return tmp;
        if(for_id === (tmp.hasAttribute("id") ? tmp.getAttribute("id") : null)) return tmp;
    }
    return null;
}
function get_label(node, show_invisible) {
    if(node.parentNode.tagName === "LABEL" && get_first_non_empty(node.parentNode) === node) return node.parentNode;
    if(show_invisible === undefined) show_invisible = false;
    var labels = document.querySelectorAll("label[for=\'" + node.getAttribute("id") + "\']");
    if(labels.length === 0) return null;
    for(var i = 0; i < labels.length; i++) {
        if((!show_invisible && !node_visible(labels[i])) || labels[i].textContent.replace(/\s{2,}/gm, ' ').trim() === "") continue;
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
function nogroup(node) {
    let class_ = (node.nodeType === Node.ELEMENT_NODE && node.hasAttribute("class")) ? node.getAttribute("class").toLowerCase() : "";
    if(class_.includes("dropdown")) return true;
    if(node.hasAttribute("data-toggle")) return true;
    return false;
}

var list = [];
let args = arguments;

function init_list(root, doc, frame) {

    if(root === null) {
        if(window.location.href.startsWith("https://web.pollpay.app")) {
            if(document.forms.length === 0) return;
            root = document.forms[0];
        }
        else {
            root = document.documentElement;
        }
    }

    var pos = 0;
    var walker = doc.createTreeWalker(root, NodeFilter.SHOW_ALL, {
        acceptNode(node, skip_add) {
            if(node === null) return NodeFilter.FILTER_ACCEPT;
            if(node.tagName === "IFRAME" || node.tagName === "FRAME") {
                if(node.contentDocument !== null) init_list(node.contentDocument.documentElement, node.contentDocument, node);
                return NodeFilter.FILTER_REJECT;
            }
            //if(node.nodeType === Node.ELEMENT_NODE && node.tagName === "LABEL" && node.hasAttribute("for") && document.querySelector("#" + node.getAttribute("for")) !== null) return NodeFilter.FILTER_REJECT;
            if(node.tagName === "OPTION") return NodeFilter.FILTER_REJECT;
            if(node.tagName === "SCRIPT") return NodeFilter.FILTER_REJECT;
            if(!node_visible(node)) {
                if(node.nodeType !== Node.ELEMENT_NODE) return NodeFilter.FILTER_ACCEPT;
                var label = get_label(node, true);
                if(label === null) return NodeFilter.FILTER_ACCEPT;
            }

            pos++;

            let tag = node.tagName;
            if(tag !== undefined) tag = tag.toUpperCase();
            var res = {};
            var skip = true;
            let class_ = (node.nodeType === Node.ELEMENT_NODE && node.hasAttribute("class")) ? node.getAttribute("class").toLowerCase() : "";
            let id = (node.nodeType === Node.ELEMENT_NODE && node.hasAttribute("id")) ? node.getAttribute("id") : "";
            let first = get_first_non_empty(node);

            var role = (node.nodeType === Node.ELEMENT_NODE && node.hasAttribute("role")) ? node.getAttribute("role").toLowerCase() : "";

            if(class_.includes("links-wrap")) return NodeFilter.FILTER_REJECT;
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
                let type = node.hasAttribute("type") ? node.getAttribute("type").toLowerCase() : "";
                if(type === "button" || type === "image") {
                    res.type = "btn";
                }
                else if(type === "checkbox" || type === "radio") {
                    res.type = "btn";
                    res.checkbox = true;
                }
                else if(type === "text" || type === "textarea" || !node.hasAttribute("type")) {
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
                else if(type === "date") {
                    res.type = "date";
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

            else if(tag === "BUTTON" || (node.tagName === "A" && node.hasAttribute("href") && !node.getAttribute("href").startsWith("/"))) {
                if(class_.includes("language")) return NodeFilter.FILTER_ACCEPT;
                if(nogroup(node)) res.nogroup = true;
                res.type = "btn";
            }

            else if(tag === "MAT-CHECKBOX" || tag === "MAT-RADIO-BUTTON" || (node.nodeType === Node.ELEMENT_NODE && node.hasAttribute("data-ng-click"))) {
                if(node.querySelector("input")) return NodeFilter.FILTER_ACCEPT;
                res.type = "btn";
                res.checkbox = true;
            }

            else if(tag === "LABEL") {
                if(node.hasAttribute("for")) {
                    var target = document.querySelector("[id=\'" + node.getAttribute("for") + "\']");
                    var first_non_empty = get_first_non_empty(node, node.getAttribute("for"));
                    if(target !== null && first_non_empty === target) return this.acceptNode(target);
                    if(target !== null) return NodeFilter.FILTER_REJECT;
                    if(first_non_empty !== null) return this.acceptNode(first_non_empty);
                    return NodeFilter.FILTER_ACCEPT;
                }
                if(node.childNodes.length > 0) {
                    return this.acceptNode(first);
                }
                return NodeFilter.FILTER_ACCEPT;
            }

            else if(tag === "DATALIST") {
                return NodeFilter.FILTER_REJECT;
            }

            else if(first !== null && (first.tagName === "INPUT" || first.tagName === "BUTTON")) {
                return NodeFilter.FILTER_ACCEPT;
            }

            else {
                const button_skip = [{tag:"UL"},{tag:"MAIN"},{tag_c:"GROUP"},{class_c:"button-bar"},{class_c:"container"},{class_c:"group"},{tag:"SVG"},{class_c:"buttons"},{class_c:"bb-"},{role_c:"group"},{id_c:"root"},{class_c:"-section"},{class_c:"wrapper"},{tag:"TABLE"},{class_c:"previous"},{tag:"TH"}];
                const button_accept = [{role:"button"},{role:"radio"},{role:"checkbox"},{attr:"ng-click"},{class_c:"btn"},{class_c:"button"},{class_c:"option-item"},{class:"grid_radio"}];
                const is_button = function (node) {
                    if(node.nodeType !== Node.ELEMENT_NODE) return false;
                    for(var i = 0; i < button_skip.length; i++) {
                        let entry = button_skip[i];
                        if(entry.tag === tag) return false;
                        if(entry.tag_c && tag.includes(entry.tag_c)) return false;
                        if(entry.class_c && class_.includes(entry.class_c)) return false;
                        if(entry.role_c && role.includes(entry.role_c)) return false;
                        if(entry.id_c && id.includes(entry.id_c)) return false;
                    }

                    if(node.onclick) return true;
                    for(var i = 0; i < button_accept.length; i++) {
                        let entry = button_accept[i];
                        if(entry.role === role) return true;
                        if(entry.attr && node.hasAttribute(entry.attr)) return true;
                        if(entry.class === class_) return true;
                        if(entry.class_c && class_.includes(entry.class_c)) return true;
                    }

                    return false;
                };
                if(is_button(node)) {
                    if(this.acceptNode(first, true) === NodeFilter.FILTER_REJECT) return NodeFilter.FILTER_ACCEPT;

                    if(nogroup(node)) res.nogroup = true;
                    res.type = "btn";
                }
                else return NodeFilter.FILTER_ACCEPT;
            }

            if(skip_add) return skip ? NodeFilter.FILTER_REJECT : NodeFilter.FILTER_ACCEPT;

            if(node.nodeType === Node.ELEMENT_NODE) {
                res.node = node;

                res.label = get_label(node, true);
                if(res.label !== null && res.label !== undefined) res.text = res.label.textContent.replace(/\s{2,}/gm, ' ').trim();
                else if(node.hasAttribute("aria-label")) res.text = node.getAttribute("aria-label");
                else if(node.hasAttribute("value")) res.text = node.getAttribute("value");
                else res.text = get_text_content(node).replace(/\s{2,}/gm, ' ').trim();
            }

            for(var i = 0; i < args.length; i++) {
                if(args[i] !== node) continue;
                res.nogroup = true;
                break;
            }

            var range = document.createRange();
            range.selectNodeContents(tag === "INPUT" ? node.parentNode : node);
            var rects = range.getClientRects();
            if(rects.length > 0) {
                res.rects = rects[0];
            }
            if(frame !== null) res.frame = frame;

            res.pos = pos;
            list.push(res);
            return skip ? NodeFilter.FILTER_REJECT : NodeFilter.FILTER_ACCEPT;
        }
    });
    while(walker.nextNode()) {}
}

init_list(null, document, null);

for(var i = 0; i < list.length; i++) {
    var current = list[i];
    if(current.type !== "btn" || current.nogroup) continue;
    var group = [i];
    var prev = i;
    i++;
    var diff = -1;
    for(;i<list.length; i++) {
        let cmp = list[i];
        if(cmp.type !== "btn" || cmp.nogroup) break;
        if(cmp.node.tagName !== current.node.tagName) {
            i--;
            break;
        }
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
for(var i = 0; i < list.length; i++) console.log(list[i]);
return list;
