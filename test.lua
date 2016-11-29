

local xml = require "lua-libxml2"


local doc = xml.loadFile("./test.xml")

local root = xml.xmlDocGetRootElement(doc)


local function dump_xml_root(root)
    if not root then
        return nil
    end

    local cur = xml.childNode(root)

    while cur do

        print("name: ", xml.getNodeName(cur), " Content: ", xml.getContent(cur))
        local attrs = xml.getAllAttributes(cur)
        for k, v in pairs(attrs) do
            print("\t", k, '-', v)
        end

        dump_xml_root(cur)

        cur = xml.nextNode(cur)
    end
end


local new = xml.xmlNewChildNode(root, "TEST");
xml.setContent(new, "HHHHH");

dump_xml_root(root)


xml.xmlFreeDoc(doc)
