
#include "lua.h"
#include "lauxlib.h"

#include <libxml/xmlreader.h>


#define CHECK(x, err) do { \
    if (x == NULL) { \
        lua_pushnil(L); \
        lua_pushstring(L, err); \
        return 2; \
    } \
} while (0)

static int lua_loadFile(lua_State *L)
{
    const char* filename = (char *) luaL_checkstring(L, 1);
    xmlDocPtr doc = xmlParseFile(filename);

    CHECK(doc, "bad filename");

    lua_pushlightuserdata(L, doc);

    return 1;
}

static int lua_xmlFreeDoc(lua_State *L)
{
    const xmlDocPtr doc = (xmlDocPtr)lua_touserdata(L, 1);

    CHECK(doc, "bad doc");

    xmlFreeDoc(doc);

    lua_pushboolean(L, 1);

    return 1;
}

static int lua_xmlDocGetRootElement(lua_State *L)
{
    xmlNode *root = NULL;
    const xmlDocPtr doc = (xmlDocPtr)lua_touserdata(L, 1);

    CHECK(doc, "bad doc");

    root = xmlDocGetRootElement(doc);

    CHECK(root, "no root");

    lua_pushlightuserdata(L, root);

    return 1;
}



static int lua_childNode(lua_State *L)
{
    xmlNodePtr xnp = (xmlNodePtr)lua_touserdata(L, 1);

    CHECK(xnp, "bad node");

    xmlNodePtr cnode = xmlFirstElementChild(xnp);
    if (cnode == NULL) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushlightuserdata(L, cnode);
    return 1;
}


static int lua_nextNode(lua_State *L)
{
    xmlNodePtr xnp = (xmlNodePtr)lua_touserdata(L, 1);

    CHECK(xnp, "bad node");

    xmlNodePtr sibling = xmlNextElementSibling(xnp);
    if (sibling == NULL) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushlightuserdata(L, sibling);
    return 1;
}

static int lua_getNodeName(lua_State *L)
{
    xmlNodePtr xnp = (xmlNodePtr)lua_touserdata(L, 1);
    CHECK(xnp, "bad node");

    const char *name = (char *)xnp->name;
    CHECK(name, "no name");

    lua_pushstring(L, name);
    return 1;
}

static int lua_getContent(lua_State *L)
{
    xmlNodePtr xnp = (xmlNodePtr)lua_touserdata(L, 1);

    CHECK(xnp, "bad node");

    char * content = (char *)xmlNodeGetContent(xnp->children);
    if (content == NULL) {
        lua_pushnil(L);
        xmlFree(content);
        return 1;
    }

    lua_pushstring(L, content);
    free(content);
    return 1;
}

static int lua_setContent(lua_State *L)
{
    xmlNodePtr xnp = (xmlNodePtr)lua_touserdata(L, 1);

    CHECK(xnp, "bad node");

    const xmlChar * content = (xmlChar *)luaL_checkstring(L, 2);
    if (content == NULL) {
        /** TODO: delete it from node **/
        lua_pushboolean(L, 1);
        return 1;
    }

    xmlNodePtr xnpc = xmlNewText(BAD_CAST content);
    xmlAddChild(xnp, xnpc);

    lua_pushboolean(L, 1);
    return 1;
}

static int lua_getAllAttributes(lua_State *L)
{
    xmlNodePtr xnp = (xmlNodePtr)lua_touserdata(L, 1);

    CHECK(xnp, "bad node");

    xmlAttr * xap = (xmlAttrPtr)xnp->properties;
    CHECK(xnp, "bad properties");

    lua_newtable(L);
    while (xap != NULL) {
        xmlChar * attrName = (xmlChar *)xap->name;
        char * attrValue = (char *)xmlGetProp(xnp, attrName);
        lua_pushstring(L, attrValue);
        lua_setfield(L, -2, (const char *)attrName);
        xap = (xmlAttr *)xap->next;
        free(attrValue);
    }

    return 1;
}


static int lua_getAttribute(lua_State *L)
{
    xmlNodePtr xnp = (xmlNodePtr)lua_touserdata(L, 1);

    CHECK(xnp, "bad node");

    const xmlChar * name = (xmlChar *)luaL_checkstring(L, 2);
    char * attr = (char *)xmlGetProp(xnp, name);

    if (attr == NULL) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushstring(L, attr);
    free(attr);
    return 1;
}


static int lua_xmlNewNode(lua_State *L)
{
    const xmlChar * name = (xmlChar *)luaL_checkstring(L, 1);
    if (name == NULL) { name = "xml"; }
    xmlNodePtr xnp = xmlNewNode(NULL, BAD_CAST name);
    CHECK(xnp, "bad new node");
    lua_pushlightuserdata(L, xnp);
    return 1;
}

static int lua_xmlNewChildNode(lua_State *L)
{
    xmlNodePtr xnp = (xmlNodePtr)lua_touserdata(L, 1);
    CHECK(xnp, "bad node");
    const xmlChar * name = (xmlChar *)luaL_checkstring(L, 2);
    if (name == NULL) { name = "xml"; }
    xmlNodePtr xnpn = xmlNewNode(NULL, BAD_CAST name);
    CHECK(xnpn, "bad new node");
    xmlAddChild(xnp, xnpn);
    lua_pushlightuserdata(L, xnpn);
    return 1;

}

static const struct luaL_Reg xmlFuncs[] = {
    {"loadFile", lua_loadFile},
    {"xmlFreeDoc", lua_xmlFreeDoc},
    {"xmlDocGetRootElement", lua_xmlDocGetRootElement},
    {"childNode", lua_childNode},
    {"nextNode", lua_nextNode},
    {"getNodeName", lua_getNodeName},
    {"getContent", lua_getContent},
    {"setContent", lua_setContent},
    {"getAttribute", lua_getAttribute},
    {"getAllAttributes", lua_getAllAttributes},
    {"xmlNewNode", lua_xmlNewNode},
    {"xmlNewChildNode", lua_xmlNewChildNode},
    {NULL, NULL}
};


int luaopen_libxml2(lua_State *L)
{
#if LUA_VERSION_NUM == 501
    luaL_register(L, "xml", xmlFuncs);
#else //lua5.2
    luaL_newlib (L, xmlFuncs);
#endif
    return 1;
}
