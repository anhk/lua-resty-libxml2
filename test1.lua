

local xml = require "lua-libxml2"

local xmlstr = '<xml>  <a> fff </a>  <b> <x name="1" value="11"/> <x name="2" value="22"/> <x name="3" value="33"/> </b> '
        .. '<c> <d> <x name="1" value="11"/> <x name="2" value="22"/> <x name="3" value="33"/> </d> '
        .. ' <e> <x name="1" value="11"> Hello World! </x> <x name="2" value="22"/> <x name="3" value="33"/> </e> </c> '
        .. ' </xml> '


local doc = xml.xmlParseDoc(xmlstr)

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
