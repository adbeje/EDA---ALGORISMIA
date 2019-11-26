#include "Player.hh"
#include <vector>
#include <queue>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>

using namespace std;

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME adbeje


struct PLAYER_NAME : public Player {
    
    /**
     * Factory: returns a new instance of this class.
     * Do not modify this function.
     */
    static Player* factory () {return new PLAYER_NAME;}
    
    /**
     * Types and attributes for your player can be defined here.
     */
    
    
    
    //Estructures d'ajut
    
    struct dPos {
        Pos p;
        int d;
        dPos(Pos pos, int dist) : p(pos), d(dist) {}
        bool operator>(const dPos& other) const { return d > other.d; }
    };
    
    //Types abreviats
    //pos
    typedef queue<Pos> QP;
    typedef vector<Pos> VP;
    typedef map<Pos,Pos> MPP;
    typedef map<Pos,int> MPE;
    
    //int
    typedef vector<int> VE;
    typedef vector<VE> VVE;
    typedef map<int,int> MEE;
    
    //bool
    typedef vector<bool> VB;
    typedef vector<VB> VVB;
    
    typedef priority_queue<dPos, vector<dPos>, greater<dPos>> PQP; 
    
    //Atributs
    VP paigua;
    VVE daigua;
    VP pesta;
    MPE proxpos;
    MPE::iterator it;
    
    /**
     * Play method, invoked once per each round.
     */
    
    
    // Devuelve las casillas vecinas a la que es posible moverse
    void veins(Pos pos, VP& res, Unit u) { // mirar de pasar la unidad en ve de un unittype
        for (int d = 0; d != DirSize; ++d) {
            Dir dir = Dir(d);
            Pos p = pos + dir;
            if (pos_ok(p) and mouguerrer(p,u) and u.type == Warrior) res.push_back(p);
            if (pos_ok(p) and moucotxe(p,u) and u.type == Car) res.push_back(p);
        }
    }
    
    inline bool comp_tipus(CellType ct, Cell c) {
        if (ct == City)
            return c.type == City and c.owner != me();
        else if (ct == Water)
            return c.type == Water;
        else if (ct == Desert)
            return c.type == Desert;
        else if (ct == Station)
            return c.type == Station;
        else if (ct == Road)
            return c.type == Road;
        _unreachable();
        
    }
    
    inline bool comp_unit(UnitType ut, Cell c) {
        if (ut == Warrior and c.id != -1)
            return unit(c.id).type == Warrior and c.owner != me();
        return false;
        _unreachable();
        
    }
    
    inline bool cel_ocupada(Pos pos) {
        if(cell(pos).id == -1) return false;
        return true;
    }
    
    
    inline bool mouguerrer(Pos p, Unit u) {
        Cell c = cell(p);
        CellType t = c.type;
        if(cel_ocupada(p)) {
            Unit u1 = unit(c.id);
            return u1.player != me() and (u1.type == Warrior and (u.water > u1.water));
        }
        else if (not cel_ocupada(p) and t != Wall and t != Station and t != Water) return true;
        return false;
        
    }
    
    
    inline bool moucotxe(Pos p, Unit u) {
        Cell c = cell(p);
        CellType t = c.type;
        if (cel_ocupada(p) and u.food > 0) {
            Unit u1 = unit(c.id);
            VE nc = cars(me());
            int n = nc.size();
            if(n < 4) return u1.player != me() and u1.type != u.type; //enviar una con 3 y una con 4
            return  u1.player != me() /*and u1.type != u.type*/;
        }
        else if (not cel_ocupada(p) and t != Wall and t != City and t != Station and t != Water) return true;
        return false;
    }
    
    
    
    
    void busca_pos(Pos pos, VP& bpos, CellType ct) {
        QP q;
        VVB visitat(rows(), VB(cols(), false));
        visitat[pos.i][pos.j] = true;
        q.push(pos);
        Pos p;
        while (not q.empty()) {
            p = q.front();
            q.pop();
            for (int d = 0; d != DirSize; ++d){
                Dir dir = Dir(d);
                Pos npos;
                if (pos_ok(p + dir)){
                    npos = p + dir;
                    if (not visitat[npos.i][npos.j]) {
                        q.push(npos);
                        visitat[npos.i][npos.j] = true;
                        if(cell(npos).type == ct) bpos.push_back(npos);
                    }
                }
            }
        }
    }
    
    
    void dijkstra_aigua(Pos p) {
        daigua[p.i][p.j] = 0;
        VVB visitat(rows(),VB(cols(),false));
        PQP pq;
        pq.push(dPos(p,0));
        dPos dp = pq.top();
        while (not pq.empty()) {
            dp = pq.top();
            pq.pop();
            for(int z = 0; z != DirSize; ++z) {
                int dd = dp.d +1;
                Dir dir = Dir(z);
                Pos npos;
                if (pos_ok(dp.p + dir)) {
                    npos = dp.p + dir;
                    if (not visitat[npos.i][npos.j]) {
                        if (dd < daigua[npos.i][npos.j]) {
                            pq.push(dPos(npos, dd));
                            daigua[npos.i][npos.j] = dd;
                        }
                        visitat[npos.i][npos.j] = true;
                    }
                }
            }
        }
    }
    
    Dir dijsktra_cotxes(Pos pos){
        Unit u = unit(cell(pos).id);
        PQP pq;
        VVE d(rows(), VE(cols(), 99999)); d[pos.i][pos.j] = 0;
        MPP pares;
        pq.push(dPos(pos, 0));
        
        dPos dp = pq.top();
        bool found = false;
        
        while (not found and not pq.empty()) {
            dp = pq.top();
            Cell c = cell(dp.p);
            if(u.food > 0  and comp_unit(Warrior, c)) found = true;
            else if(u.food == 0 and comp_tipus(Station,c)) found = true;
            
            else {
                pq.pop();
                
                VP nvei;
                veins(dp.p, nvei, u);
                int nv = nvei.size();
                for (int i = 0; i < nv; ++i) {
                    int dd = dp.d +1;
                    Pos n = nvei[i];
                    if (comp_tipus(Desert,cell(n)) and ((round() + dd)%4) != (me()%4)) dd = dd + 4;
                    if (dd < d[n.i][n.j]) {
                        pq.push(dPos(n, dd));
                        d[n.i][n.j] = dd;
                        pares[n] = dp.p;
                    }
                }
            }
        }
        
        Pos p = dp.p;
        
        while (p != pos and pares[p] != pos) p = pares[p];
        if (p == pos) return Dir(None);
        
        // Calculo la direccio inicial
        for (int d = 0; d != DirSize; ++d) {
            Dir dir = Dir(d);
            Pos npos = pos + dir;
            if (npos == p) {
                return dir;
            }
        }
        
        _unreachable();
    }

    
    Dir bfs_guerrers (Pos pos) {
        Unit u = unit(cell(pos).id);
        QP q;
        VVB visitat(rows(), VB(cols(), false));
        MPP pares;
        visitat[pos.i][pos.j] = true;
        q.push(pos);
        
        Pos p;
        bool found = false;
        
        while (not q.empty() and not found) {
            p = q.front();
            Cell c = cell(p);
            if(comp_tipus(City, c)) found = true;
            else {
                q.pop();
                VP nvei;
                veins(p, nvei, u);
                int nv = nvei.size();
                for (int i = 0; i < nv; ++i) {
                    Pos n = nvei[i];
                    if (not visitat[n.i][n.j]) {
                        q.push(n);
                        pares[n] = p;
                        visitat[n.i][n.j] = true;
                    }
                }
            }
        }
        while (p != pos and pares[p] != pos) p = pares[p];
        if (p == pos) return Dir(None);
        
        // Calculo la direccio inicial
        for (int d = 0; d != DirSize; ++d) {
            Dir dir = Dir(d);
            Pos npos = pos + dir;
            if (npos == p) {
                return dir;
            }
        }
        
        _unreachable();
    }
    
    Dir tinc_sed(Pos& pos) {
        int val_min = daigua[pos.i][pos.j];
        Dir dir = None;
        for (int d = 0; d != DirSize; ++d) {
            Pos p = pos + Dir(d);
            if(pos_ok(p) and daigua[p.i][p.j] < val_min) {
                val_min = daigua[p.i][p.j];
                dir = Dir(d);
            }
        }
        return dir;
    }
    
    void mou_guerrer(int id) {
        Unit u = unit(id);
        Dir dg;
        if(u.water < 15) dg = tinc_sed(u.pos);
        else dg = bfs_guerrers(u.pos); 
        command(id, dg);
    } 

    
    void mou_cotxe(int id) {
        Unit u = unit(id);
        Dir dc = dijsktra_cotxes(u.pos);
        it = proxpos.find(u.pos+Dir(dc));
        if(it != proxpos.end()) command(id, None);
        else {
            proxpos[u.pos+Dir(dc)] = id;
            command(id, dc);
        }
    }
    
    
    
    virtual void play () {
        
        if(round() == 0) {
            //mapa aigua
            daigua = VVE(rows(), VE(cols(), 99999));
            busca_pos(Pos(29,29),paigua,Water);
            int n = paigua.size();
            for (int i = 0; i < n; ++i) {
                dijkstra_aigua(paigua[i]);
            }
            
        }  
        
        VE cotxes = cars(me());
        for (int id : cotxes) {
            if(can_move(id)) {
                mou_cotxe(id);
                
            }
        }
        proxpos.clear();
        
        VE guerrers = warriors(me());
        for (int id : guerrers) {
            if(can_move(id)){
                mou_guerrer(id);
            }
        }
    }
    
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
