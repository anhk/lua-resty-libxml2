
#include "lua.h"
#include "lauxlib.h"

#include <string.h>
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

static int lua_xmlParseDoc(lua_State *L)
{
    const char* string = (char *)luaL_checkstring(L, 1);

    xmlInitParser();
    xmlDocPtr doc = xmlParseDoc((const xmlChar *)string);

    CHECK(doc, "bad string");

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

static int lua_xmlNewDoc(lua_State *L)
{
    const xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    CHECK(doc, "bad doc");

    lua_pushlightuserdata(L, doc);
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

static int lua_xmlDocSetRootElement(lua_State *L)
{
    const xmlDocPtr doc = (xmlDocPtr)lua_touserdata(L, 1);
    CHECK(doc, "bad doc");

    const xmlNodePtr xnp = (xmlNodePtr)lua_touserdata(L, 2);
    CHECK(xnp, "bad node");

    xmlDocSetRootElement(doc, xnp);

    lua_pushboolean(L, 1);
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

static xmlNodePtr lua_findChildNode(xmlNodePtr root, const char *name)
{
    xmlNodePtr result = xmlFirstElementChild(root);

    while (result) {
        if (strcmp(result->name, name) == 0) {
            return result;
        }
        result = xmlNextElementSibling(result);
    }
    return NULL;
}

static int lua_findNode(lua_State *L)
{
    xmlNodePtr root = (xmlNodePtr)lua_touserdata(L, 1);
    CHECK(root, "bad node");
    xmlNodePtr result = NULL;

    int argc = lua_gettop(L);

    const char* name1 = (char *)luaL_checkstring(L, 2);
    const char* name2 = NULL;
    const char* name3 = NULL;

    if (name1 == NULL) {
        goto out;
    }

    result = lua_findChildNode(root, name1);
    if (argc <= 2 || result == NULL) {
        goto out;
    }

    name2 = (char *)luaL_checkstring(L, 3);
    result = lua_findChildNode(result, name2);
    if (argc <= 3 || result == NULL) {
        goto out;
    }

    name3 = (char *)luaL_checkstring(L, 4);
    result = lua_findChildNode(result, name3);

out:
    if (result) {
        lua_pushlightuserdata(L, result);
    } else {
        lua_pushnil(L);
    }
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
    const xmlChar * name = NULL;
    int argc = lua_gettop(L);

    if (argc >= 1) {
        name = (xmlChar *)luaL_checkstring(L, 1);
    }
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

static int lua_xmlNewCDataChildNode(lua_State *L)
{
    const xmlDocPtr doc = (xmlDocPtr)lua_touserdata(L, 1);
    CHECK(doc, "bad doc");
    xmlNodePtr root = (xmlNodePtr)lua_touserdata(L, 2);
    CHECK(root, "bad node");
    const xmlChar * name = (xmlChar *)luaL_checkstring(L, 3);
    CHECK(name, "bad name");
    const xmlChar * content = (xmlChar *)luaL_checkstring(L, 4);
    CHECK(name, "bad content");
    xmlNodePtr xnp = xmlNewNode(NULL, BAD_CAST name);
    CHECK(xnp, "bad new node");
    xmlNodePtr xnpn = xmlNewCDataBlock(doc, content, strlen(content));
    CHECK(xnpn, "bad new cdata node");
    xmlAddChild(root, xnp);
    xmlAddChild(xnp, xnpn);
    lua_pushlightuserdata(L, xnp);
    return 1;
}

static int lua_xmlDocDump(lua_State *L)
{
    const xmlDocPtr doc = (xmlDocPtr)lua_touserdata(L, 1);
    CHECK(doc, "bad doc");

    xmlBufferPtr nodeBuffer = xmlBufferCreate();
    CHECK(nodeBuffer, "bad buffer");

    xmlNodePtr root = xmlDocGetRootElement(doc);
    CHECK(root, "bad node");

    if (xmlNodeDump(nodeBuffer, doc, root, 0, 1) > 0) {
        lua_pushstring(L, nodeBuffer->content);
    } else {
        lua_pushnil(L);
    }
    xmlBufferFree(nodeBuffer);

    return 1;

#if 0
    xmlNodePtr xnp = (xmlNodePtr)lua_touserdata(L, 1);
    CHECK(xnp, "bad node");
    if(xmlNodeDump(nodeBuffer,doc,curNode, 0, 1) > 0){
        printf("%s\n",(char *)nodeBuffer->content);
        printf("use:%d\nsize:%d\n",nodeBuffer->use,nodeBuffer->size);
    }
    xmlBufferFree(nodeBuffer);
#endif
}

static const struct luaL_Reg xmlFuncs[] = {
    {"loadFile", lua_loadFile},
    {"xmlParseDoc", lua_xmlParseDoc},
    {"xmlFreeDoc", lua_xmlFreeDoc},
    {"xmlNewDoc", lua_xmlNewDoc},
    {"xmlDocGetRootElement", lua_xmlDocGetRootElement},
    {"xmlDocSetRootElement", lua_xmlDocSetRootElement},
    {"childNode", lua_childNode},
    {"nextNode", lua_nextNode},
    {"findNode", lua_findNode},
    {"getNodeName", lua_getNodeName},
    {"getContent", lua_getContent},
    {"setContent", lua_setContent},
    {"getAttribute", lua_getAttribute},
    {"getAllAttributes", lua_getAllAttributes},
    {"xmlNewNode", lua_xmlNewNode},
    {"xmlNewChildNode", lua_xmlNewChildNode},
    {"xmlNewCDataChildNode", lua_xmlNewCDataChildNode},
    {"xmlDocDump", lua_xmlDocDump},
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
