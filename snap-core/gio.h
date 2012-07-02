/////////////////////////////////////////////////
// Loading and saving graphs from/to various file formats.
namespace TSnap {

/// Loads a (directed, undirected or multi) graph from a text file InFNm with 1 edge per line (whitespace separated columns, integer node ids).
template <class PGraph> PGraph LoadEdgeList(const TStr& InFNm, const int& SrcColId=0, const int& DstColId=1);
/// Loads a (directed, undirected or multi) graph from a text file InFNm with 1 edge per line ('Separtor' separated columns, integer node ids).
template <class PGraph> PGraph LoadEdgeList(const TStr& InFNm, const int& SrcColId, const int& DstColId, const char& Separator);
/// Loads a (directed, undirected or multi) graph from a text file InFNm with 1 edge per line (whitespace separated columns, arbitrary string node ids).
template <class PGraph> PGraph LoadEdgeListStr(const TStr& InFNm, const int& SrcColId=0, const int& DstColId=1);
/// Loads a (directed, undirected or multi) graph from a text file InFNm with 1 edge per line (whitespace separated columns, arbitrary string node ids).
template <class PGraph> PGraph LoadEdgeListStr(const TStr& InFNm, const int& SrcColId, const int& DstColId, TStrHash<TInt>& StrToNIdH);
/// Loads a (directed, undirected or multi) graph from a text file InFNm with 1 node and all its edges in a single line.
template <class PGraph> PGraph LoadConnList(const TStr& InFNm);

/// Loads a (directed, undirected or multi) graph from Pajek .NET file.
template <class PGraph> PGraph LoadPajek(const TStr& InFNm);
/// Loads a directed network in the DyNetML format. Loads only the first network in the file FNm.
PNGraph LoadDyNet(const TStr& FNm);
/// Loads directed networks in the DyNetML format. Loads all the networks in the file FNm.
TVec<PNGraph> LoadDyNetGraphV(const TStr& FNm);

//TODO: Sparse/Dense adjacency matrix which values we threshold at Thresh to obtain an adjacency matrix.
//template <class PGraph> PGraph LoadAdjMtx(const TStr& FNm, const int Thresh);
//TODO: Load from a GML file format (http://en.wikipedia.org/wiki/Graph_Modelling_Language)
//template <class PGraph> PGraph LoadGml(const TStr& FNm, const int Thresh);


/// Saves a graph into a text file. Each line contains two columsn and encodes a single edge: <source node id><tab><destination node id>
template <class PGraph> void SaveEdgeList(const PGraph& Graph, const TStr& OutFNm, const TStr& Desc=TStr());
/// Saves a graph in a Pajek .NET format.
template <class PGraph> void SavePajek(const PGraph& Graph, const TStr& OutFNm);
/// Saves a graph in a Pajek .NET format.
template <class PGraph> void SavePajek(const PGraph& Graph, const TStr& OutFNm, const TIntStrH& NIdColorH);
/// Saves a graph in a Pajek .NET format.
template <class PGraph> void SavePajek(const PGraph& Graph, const TStr& OutFNm, const TIntStrH& NIdColorH, const TIntStrH& NIdLabelH);
/// Saves a graph in a Pajek .NET format.
template <class PGraph> void SavePajek(const PGraph& Graph, const TStr& OutFNm, const TIntStrH& NIdColorH, const TIntStrH& NIdLabelH, const TIntStrH& EIdColorH);
/// Saves a graph in a MATLAB sparse matrix format.
template <class PGraph> void SaveMatlabSparseMtx(const PGraph& Graph, const TStr& OutFNm);

//TODO:  Save to a GML file format (http://en.wikipedia.org/wiki/Graph_Modelling_Language)
//template <class PGraph> SaveGml(const PGraph& Graph, const TStr& OutFNm, const TStr& Desc);

/////////////////////////////////////////////////
// Implementation

/// Whitespace separated file of several columns: ... <source node id> ... <destination node id> ...
/// SrcColId and DstColId are column indexes of source/destination (integer!) node ids.
/// This means there is one edge per line and node IDs are assumed to be integers.
/// The function loads the format saved by TSnap::SaveEdgeList()
template <class PGraph>
PGraph LoadEdgeList(const TStr& InFNm, const int& SrcColId, const int& DstColId) {
  TSsParser Ss(InFNm, ssfWhiteSep, true, true, true);
  PGraph Graph = PGraph::TObj::New();
  int SrcNId, DstNId;
  while (Ss.Next()) {
    if (! Ss.GetInt(SrcColId, SrcNId) || ! Ss.GetInt(DstColId, DstNId)) { continue; }
    if (! Graph->IsNode(SrcNId)) { Graph->AddNode(SrcNId); }
    if (! Graph->IsNode(DstNId)) { Graph->AddNode(DstNId); }
    Graph->AddEdge(SrcNId, DstNId);
  }
  Graph->Defrag();
  return Graph;
}

/// 'Separator' separated file of several columns: ... <source node id> ... <destination node id> ...
/// SrcColId and DstColId are column indexes of source/destination (integer!) node ids.
/// This means there is one edge per line and node IDs are assumed to be integers.
/// The function loads the format saved by TSnap::SaveEdgeList() if we set Separator='\t'.
template <class PGraph>
PGraph LoadEdgeList(const TStr& InFNm, const int& SrcColId, const int& DstColId, const char& Separator) {
  TSsParser Ss(InFNm, Separator);
  PGraph Graph = PGraph::TObj::New();
  int SrcNId, DstNId;
  while (Ss.Next()) {
    if (! Ss.GetInt(SrcColId, SrcNId) || ! Ss.GetInt(DstColId, DstNId)) { continue; }
    if (! Graph->IsNode(SrcNId)) { Graph->AddNode(SrcNId); }
    if (! Graph->IsNode(DstNId)) { Graph->AddNode(DstNId); }
    Graph->AddEdge(SrcNId, DstNId);
  }
  Graph->Defrag();
  return Graph;
}

/// Whitespace separated file of several columns: ... <source node id> ... <destination node id> ...
/// SrcColId and DstColId are column indexes of source/destination (string) node ids.
/// This means there is one edge per line and node IDs can be arbitrary STRINGs. 
/// Note that the mapping of node names to ids is discarded.
template <class PGraph>
PGraph LoadEdgeListStr(const TStr& InFNm, const int& SrcColId, const int& DstColId) {
  TSsParser Ss(InFNm, ssfWhiteSep);
  PGraph Graph = PGraph::TObj::New();
  TStrHash<TInt> StrToNIdH(Mega(1), true); // hash-table mapping strings to integer node ids
  while (Ss.Next()) {
    const int SrcNId = StrToNIdH.AddKey(Ss[SrcColId]);
    const int DstNId = StrToNIdH.AddKey(Ss[DstColId]);
    if (! Graph->IsNode(SrcNId)) { Graph->AddNode(SrcNId); }
    if (! Graph->IsNode(DstNId)) { Graph->AddNode(DstNId); }
    Graph->AddEdge(SrcNId, DstNId);
  }
  Graph->Defrag();
  return Graph;
}

/// Whitespace separated file of several columns: ... <source node id> ... <destination node id> ...
/// SrcColId and DstColId are column indexes of source/destination (string) node ids.
/// This means there is one edge per line and node IDs can be arbitrary STRINGs.
/// The mapping of strings to node ids in stored in StrToNIdH.
/// To map between node names and ids use: NId = StrToNIdH.GetKeyId(NodeName) and TStr NodeName = StrToNIdH[NId];
template <class PGraph>
PGraph LoadEdgeListStr(const TStr& InFNm, const int& SrcColId, const int& DstColId, TStrHash<TInt>& StrToNIdH) {
  TSsParser Ss(InFNm, ssfWhiteSep);
  PGraph Graph = PGraph::TObj::New();
  while (Ss.Next()) {
    const int SrcNId = StrToNIdH.AddKey(Ss[SrcColId]);
    const int DstNId = StrToNIdH.AddKey(Ss[DstColId]);
    if (! Graph->IsNode(SrcNId)) { Graph->AddNode(SrcNId); }
    if (! Graph->IsNode(DstNId)) { Graph->AddNode(DstNId); }
    Graph->AddEdge(SrcNId, DstNId);
  }
  Graph->Defrag();
  return Graph;
}

/// Whitespace separated file of several columns: <source node id> <destination node id1> <destination node id2> ... 
/// First colum of each line contains a source node id followed by ids of the destination nodes.
/// For example, '1 2 3' encodes edges 1-->2 and 1-->3. Note that this format allows for saving isolated nodes.
template <class PGraph>
PGraph LoadConnList(const TStr& InFNm) {
  TSsParser Ss(InFNm, ssfWhiteSep);
  PGraph Graph = PGraph::TObj::New();
  while (Ss.Next()) {
    if (! Ss.IsInt(0)) { continue; }
    const int SrcNId = Ss.GetInt(0);
    if (! Graph->IsNode(SrcNId)) { Graph->AddNode(SrcNId); }
    for (int dst = 1; dst < Ss.Len(); dst++) {
      const int DstNId = Ss.GetInt(dst);
      if (! Graph->IsNode(DstNId)) { Graph->AddNode(DstNId); }
      Graph->AddEdge(SrcNId, DstNId);
    }
  }
  Graph->Defrag();
  return Graph;
}

template <class PGraph>
PGraph LoadPajek(const TStr& InFNm) {
  PGraph Graph = PGraph::TObj::New();
  TSsParser Ss(InFNm, ssfSpaceSep, true, true);
  Ss.Next();  Ss.ToLc();
  // nodes
  EAssert(strstr(Ss[0], "*vertices") != NULL);
  while (Ss.Next()) {
    Ss.ToLc();
    if (strstr(Ss[0], "*arcs")!=NULL || strstr(Ss[0],"*edges")!=NULL) { break; } // arcs are directed, edges are undirected
    Graph->AddNode(Ss.GetInt(0));
  }
  // edges
  while (Ss.Next()) {
    Graph->AddEdge(Ss.GetInt(0), Ss.GetInt(1));
  }
  return Graph;
}

template <class PGraph>
void SaveEdgeList(const PGraph& Graph, const TStr& OutFNm, const TStr& Desc) {
  FILE *F = fopen(OutFNm.CStr(), "wt");
  if (HasGraphFlag(typename PGraph::TObj, gfDirected)) { fprintf(F, "# Directed graph: %s \n", OutFNm.CStr()); } 
  else { fprintf(F, "# Undirected graph (each unordered pair of nodes is saved once): %s\n", OutFNm.CStr()); }
  if (! Desc.Empty()) { fprintf(F, "# %s\n", Desc.CStr()); }
  fprintf(F, "# Nodes: %d Edges: %d\n", Graph->GetNodes(), Graph->GetEdges());
  if (HasGraphFlag(typename PGraph::TObj, gfDirected)) { fprintf(F, "# FromNodeId\tToNodeId\n"); }
  else { fprintf(F, "# NodeId\tNodeId\n"); }
  for (typename PGraph::TObj::TEdgeI ei = Graph->BegEI(); ei < Graph->EndEI(); ei++) {
    fprintf(F, "%d\t%d\n", ei.GetSrcNId(), ei.GetDstNId());
  }
  fclose(F);
}

template <class PGraph>
void SavePajek(const PGraph& Graph, const TStr& OutFNm) {
  TIntH NIdToIdH(Graph->GetNodes(), true);
  FILE *F = fopen(OutFNm.CStr(), "wt");
  fprintf(F, "*Vertices %d\n", Graph->GetNodes());
  int i = 0;
  for (typename PGraph::TObj::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++, i++) {
    fprintf(F, "%d  \"%d\" ic Red fos 10\n", i+1, NI.GetId()); // ic: internal color, fos: font size
    NIdToIdH.AddDat(NI.GetId(), i+1);
  }
  if (HasGraphFlag(typename PGraph::TObj, gfDirected)) {
    fprintf(F, "*Arcs %d\n", Graph->GetEdges()); } // arcs are directed, edges are undirected
  else {
    fprintf(F, "*Edges %d\n", Graph->GetEdges());
  }
  for (typename PGraph::TObj::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
    const int SrcNId = NIdToIdH.GetDat(EI.GetSrcNId());
    const int DstNId = NIdToIdH.GetDat(EI.GetDstNId());
    fprintf(F, "%d %d %d c Black\n", SrcNId, DstNId, 1); // width=1
  }
  fclose(F);
}

/// NIdColorH maps node ids to node colors. Default node color is Red.
/// See http://vlado.fmf.uni-lj.si/pub/networks/pajek/doc/pajekman.pdf for a list of supported color names.
template <class PGraph>
void SavePajek(const PGraph& Graph, const TStr& OutFNm, const TIntStrH& NIdColorH) {
  TIntH NIdToIdH(Graph->GetNodes(), true);
  FILE *F = fopen(OutFNm.CStr(), "wt");
  fprintf(F, "*Vertices %d\n", Graph->GetNodes());
  int i = 0;
  for (typename PGraph::TObj::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++, i++) {
    fprintf(F, "%d  \"%d\" ic %s fos 10\n", i+1, NI.GetId(),
      NIdColorH.IsKey(NI.GetId()) ? NIdColorH.GetDat(NI.GetId()).CStr() : "Red");
    NIdToIdH.AddDat(NI.GetId(), i+1);
  }
  if (HasGraphFlag(typename PGraph::TObj, gfDirected)) {
    fprintf(F, "*Arcs %d\n", Graph->GetEdges()); } // arcs are directed, edges are undirected
  else {
    fprintf(F, "*Edges %d\n", Graph->GetEdges());
  }
  for (typename PGraph::TObj::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
    const int SrcNId = NIdToIdH.GetDat(EI.GetSrcNId());
    const int DstNId = NIdToIdH.GetDat(EI.GetDstNId());
    fprintf(F, "%d %d %d c Black\n", SrcNId, DstNId, 1);
  }
  fclose(F);
}

/// NIdColorH maps node ids to node colors. Default node color is Red.
/// NIdLabelH maps node ids to node string labels.
/// See http://vlado.fmf.uni-lj.si/pub/networks/pajek/doc/pajekman.pdf for a list of supported color names.
template <class PGraph>
void SavePajek(const PGraph& Graph, const TStr& OutFNm, const TIntStrH& NIdColorH, const TIntStrH& NIdLabelH) {
  TIntH NIdToIdH(Graph->GetNodes(), true);
  FILE *F = fopen(OutFNm.CStr(), "wt");
  fprintf(F, "*Vertices %d\n", Graph->GetNodes());
  int i = 0;
  for (typename PGraph::TObj::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++, i++) {
    fprintf(F, "%d  \"%s\" ic %s fos 10\n", i+1,
      NIdLabelH.IsKey(NI.GetId()) ? NIdLabelH.GetDat(NI.GetId()).CStr() : TStr::Fmt("%d", NI.GetId()).CStr(),
      NIdColorH.IsKey(NI.GetId()) ? NIdColorH.GetDat(NI.GetId()).CStr() : "Red");
    NIdToIdH.AddDat(NI.GetId(), i+1);
  }
  if (HasGraphFlag(typename PGraph::TObj, gfDirected)) {
    fprintf(F, "*Arcs %d\n", Graph->GetEdges()); } // arcs are directed, edges are undirected
  else {
    fprintf(F, "*Edges %d\n", Graph->GetEdges());
  }
  for (typename PGraph::TObj::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
    const int SrcNId = NIdToIdH.GetDat(EI.GetSrcNId());
    const int DstNId = NIdToIdH.GetDat(EI.GetDstNId());
    fprintf(F, "%d %d %d c Black\n", SrcNId, DstNId, 1);
  }
  fclose(F);
}

/// NIdColorH maps node ids to node colors. Default node color is Red.
/// NIdLabelH maps node ids to node string labels.
/// EIdColorH maps edge ids to node colors. Default edge color is Black.
/// See http://vlado.fmf.uni-lj.si/pub/networks/pajek/doc/pajekman.pdf for a list of supported color names.
template <class PGraph>
void SavePajek(const PGraph& Graph, const TStr& OutFNm, const TIntStrH& NIdColorH, const TIntStrH& NIdLabelH, const TIntStrH& EIdColorH) {
  CAssert(HasGraphFlag(typename PGraph::TObj, gfMultiGraph)); // network needs to have edge ids
  TIntH NIdToIdH(Graph->GetNodes(), true);
  FILE *F = fopen(OutFNm.CStr(), "wt");
  fprintf(F, "*Vertices %d\n", Graph->GetNodes());
  int i = 0;
  for (typename PGraph::TObj::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++, i++) {
    fprintf(F, "%d  \"%s\" ic %s fos 10\n", i+1,
      NIdLabelH.IsKey(NI.GetId()) ? NIdLabelH.GetDat(NI.GetId()).CStr() : TStr::Fmt("%d", NI.GetId()).CStr(),
      NIdColorH.IsKey(NI.GetId()) ? NIdColorH.GetDat(NI.GetId()).CStr() : "Red");
    NIdToIdH.AddDat(NI.GetId(), i+1);
  }
  if (HasGraphFlag(typename PGraph::TObj, gfDirected)) {
    fprintf(F, "*Arcs %d\n", Graph->GetEdges()); } // arcs are directed, edges are undirected
  else {
    fprintf(F, "*Edges %d\n", Graph->GetEdges());
  }
  for (typename PGraph::TObj::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
    const int SrcNId = NIdToIdH.GetDat(EI.GetSrcNId());
    const int DstNId = NIdToIdH.GetDat(EI.GetDstNId());
    fprintf(F, "%d %d 1 c %s\n", SrcNId, DstNId, 1,
      EIdColorH.IsKey(EI.GetId()) ? EIdColorH.GetDat(EI.GetId()).CStr() : "Black");
  }
  fclose(F);
}

/// Each line contains a tuple of 3 values: <source node id><tab><destination node id><tab>1
template <class PGraph>
void SaveMatlabSparseMtx(const PGraph& Graph, const TStr& OutFNm) {
  FILE *F = fopen(OutFNm.CStr(), "wt");
  TIntSet NIdSet(Graph->GetNodes()); // so that
  for (typename PGraph::TObj::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    NIdSet.AddKey(NI.GetId());
  }
  for (typename PGraph::TObj::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
    const int Src = NIdSet.GetKeyId(EI.GetSrcNId())+1;
    const int Dst = NIdSet.GetKeyId(EI.GetDstNId())+1;
    fprintf(F, "%d\t%d\t1\n", Src, Dst);
    if (! HasGraphFlag(typename PGraph::TObj, gfDirected) && Src!=Dst) {
      fprintf(F, "%d\t%d\t1\n", Dst, Src);
    }
  }
  fclose(F);
}

} // namespace TSnap