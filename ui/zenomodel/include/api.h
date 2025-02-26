#ifndef __ZENO_MODEL_API_H__
#define __ZENO_MODEL_API_H__

#include "enum.h"

//io
ZENO_ERROR  Zeno_NewFile();
ZENO_ERROR  Zeno_OpenFile(const std::string& fn);
ZENO_ERROR  Zeno_SaveAs(const std::string& fn);

ZENO_HANDLE Zeno_CreateGraph(const std::string& name);
ZENO_ERROR  Zeno_DeleteGraph(ZENO_HANDLE hSubgraph);
ZENO_HANDLE Zeno_GetGraph(const std::string& name);
ZENO_ERROR  Zeno_RenameGraph(ZENO_HANDLE hSubgraph, const std::string& newName);

ZENO_ERROR  Zeno_ForkGraph(
        ZENO_HANDLE hSubgWhere,
        const std::string& name,
        ZENO_HANDLE& hForkedSubg,
        ZENO_HANDLE& hForkedNode
);

int         Zeno_GetCount();
ZENO_HANDLE Zeno_GetItem(int idx);

ZENO_HANDLE Zeno_AddNode(ZENO_HANDLE hGraph, const std::string &nodeCls);

ZENO_ERROR  Zeno_DeleteNode(ZENO_HANDLE hNode);

ZENO_ERROR  Zeno_GetName(ZENO_HANDLE hNode, /*out*/ std::string& ret);

ZENO_ERROR  Zeno_AddLink(
        ZENO_HANDLE hOutnode,
        const std::string& outSock,
        ZENO_HANDLE hInnode,
        const std::string& inSock
);

ZENO_ERROR  Zeno_RemoveLink(
        ZENO_HANDLE hOutnode,
        const std::string& outSock,
        ZENO_HANDLE hInnode,
        const std::string& inSock
);

ZENO_ERROR  Zeno_GetOutNodes(
        ZENO_HANDLE hNode,
        const std::string& outSock,
        /*out*/ std::vector<std::pair<ZENO_HANDLE, std::string>>& res
);

ZENO_ERROR  Zeno_GetInput(
        ZENO_HANDLE hNode,
        const std::string& inSock,
        /*out*/ std::pair<ZENO_HANDLE, std::string>& ret
);

ZENO_ERROR  Zeno_GetInputDefl(
        ZENO_HANDLE hNode,
        const std::string& inSock,
        /*out*/ ZVARIANT& ret,
        /*out*/ std::string& type
);

ZENO_ERROR  Zeno_SetInputDefl(
        ZENO_HANDLE hNode,
        const std::string& inSock,
        const ZVARIANT& var
);

ZENO_ERROR  Zeno_GetParam(
        ZENO_HANDLE hNode,
        const std::string& name,
        /*out*/ ZVARIANT& ret,
        /*out*/ std::string& type
);

ZENO_ERROR  Zeno_SetParam(
        ZENO_HANDLE hNode,
        const std::string& name,
        const ZVARIANT& var
);

ZENO_ERROR Zeno_IsView(ZENO_HANDLE hNode, bool& ret);
ZENO_ERROR Zeno_SetView(ZENO_HANDLE hNode, bool bOn);
ZENO_ERROR Zeno_IsMute(ZENO_HANDLE hNode, bool& ret);
ZENO_ERROR Zeno_SetMute(ZENO_HANDLE hNode, bool bOn);
ZENO_ERROR Zeno_IsOnce(ZENO_HANDLE hNode, bool& ret);
ZENO_ERROR Zeno_SetOnce(ZENO_HANDLE hNode, bool bOn);
ZENO_ERROR Zeno_GetPos(ZENO_HANDLE hNode, std::pair<float, float>& pt);
ZENO_ERROR Zeno_SetPos(ZENO_HANDLE hNode, const std::pair<float, float>& pt);

#endif