use std::collections::BTreeMap;
use std::fmt::Debug;
use std::hash::Hash;
use std::cmp::Eq;
use rand::distributions::weighted::WeightedIndex;
use rand::distributions::Distribution;
use rand::Rng;
use rand::thread_rng;

#[derive(Clone, Ord, Eq, PartialEq, PartialOrd)]
enum Symbol<T: Clone + Debug + Eq + Hash + PartialEq + Ord> {
    Start,
    End,
    Sym(T),
}

impl<T: Clone + Debug + Eq + Hash + PartialEq + Ord> Debug for Symbol<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> Result<(), std::fmt::Error> {
        match self {
            Symbol::Start => write!(f, "Start"),
            Symbol::End => write!(f, "End"),
            Symbol::Sym(s) => write!(f, "{:?}", s),
        }?;

        Ok(())
    }
}

impl<T: Clone + Debug + Eq + Hash + PartialEq + Ord> From<T> for Symbol<T> {
    fn from(other: T) -> Symbol<T> {
        Symbol::Sym(other)
    }
}

type SymbolString<T> = Vec<Symbol<T>>;

fn get_ngrams<T: Clone + Debug + Eq + Hash + PartialEq + Ord>(s: &Vec<T>, n: usize) -> Vec<SymbolString<T>> {
    if s.len() < n {return vec![];}
    let mut ngrams = vec![];

    let mut start_str = vec![Symbol::Start];
    start_str.extend(
        s[0 .. (n-1)].to_vec()
        .into_iter().map(|s| Symbol::Sym(s))
    );
    ngrams.push(start_str);

    let mut end_str = vec![];
    end_str.extend(
        s[s.len() - n + 1 .. s.len()].to_vec()
        .into_iter().map(|s| Symbol::Sym(s))
    );
    end_str.push(Symbol::End);

    ngrams.push(end_str);

    for i in 0..(s.len() - n + 1) {
        ngrams.push(
            s[i..(i+n)].to_vec()
            .into_iter().map(|s| Symbol::Sym(s))
            .collect()
        );
    }

    ngrams
}

#[derive(Debug, Clone)]
struct MarkovState<T: Clone + Debug + Eq + PartialEq + Ord + Hash> {
    next_states: Vec<Symbol<T>>,
    dist: WeightedIndex<usize>,
}

#[derive(Debug)]
struct Markov<T: Clone + Debug + Eq + PartialEq + Hash + Ord> {
    states: BTreeMap<SymbolString<T>, MarkovState<T>>,
    max_order: usize,
}

impl<T: Clone + Debug + Eq + PartialEq + Hash + Ord> Markov<T> {
    pub fn new(max_order: usize, sequences: &[Vec<T>]) -> Self {
        let mut tree = BTreeMap::new();
        for seq in sequences {
            Self::process_sequence(&mut tree, &seq, max_order + 1);
        }
        //println!("{:?}", tree);
        
        let mut markov = Self {
            states: BTreeMap::new(),
            max_order
        };
        
        for (state, counters) in tree {
            let mut next_states = vec![];
            let mut next_weights = vec![];
            
            for (next, cnt) in counters {
                next_states.push(next);
                next_weights.push(cnt);
            }
            
            markov.states.insert(state.clone(), MarkovState{
                next_states,
                dist: WeightedIndex::new(&next_weights).unwrap()
            });
        }
        
        markov
    }

    fn process_sequence(tree: &mut BTreeMap<SymbolString<T>, BTreeMap<Symbol<T>, usize>>, s: &Vec<T>, order: usize) {
        for n in 2..=order {
            for ngram in get_ngrams(s, n) {
                let state = ngram[0 .. n - 1].to_vec();
                let next = &ngram[n - 1];

                *tree
                    .entry(state)
                    .or_insert(BTreeMap::new())
                    .entry(next.clone())
                    .or_insert(0)
                    += 1;
            }
        }
    }

    fn sample_state<R: Rng + ?Sized>(&self, state: &SymbolString<T>, rng: &mut R) -> Option<T> {
        let st = self.states.get(state)?;
        let id = st.dist.sample(rng);
        let next = st.next_states[id].clone();
        match next {
            Symbol::Start => panic!("Start symbol in the middle!"),
            Symbol::End => None,
            Symbol::Sym(t) => Some(t),
        }
    }
    
    pub fn generate<R: Rng + ?Sized>(&self, rng: &mut R, order: usize, max_len: usize) -> Vec<T> {
        assert!(order <= self.max_order);
        
        let mut result = Vec::new();
        let mut state = Vec::<Symbol<T>>::new();
        state.push(Symbol::Start);
        
        while result.len() < max_len {
            //println!("State is: {:?}", state);
            
            if let Some(next) = self.sample_state(&state, rng) {
                //println!("Next is: {:?}", next);
                state.push(Symbol::Sym(next.clone()));
                result.push(next);
                
                if state.len() > order {
                    state = state.into_iter().rev().take(order).rev().collect();
                }
            }
            else {
                break;
            }
        }
        
        result 
    }
    
}

#[derive(Debug)]
struct StringMarkov {
    markov: Markov<char>
}

impl StringMarkov{
    pub fn new(max_order: usize, sequences: &[String]) -> Self {
        let vecs: Vec<Vec<char>> = sequences.iter().map(|s| s.chars().collect()).collect();
        StringMarkov {
            markov: Markov::new(max_order, &vecs)
        }
    }
    
     pub fn generate<R: Rng + ?Sized>(&self, rng: &mut R, order: usize, max_len: usize) -> String {
         self.markov.generate(rng, order, max_len).into_iter().collect()
     }
}

fn main() {

    let strs = vec![
        "neptune",
        "uranus",
        "saturn",
        "earth",
        "mercury",
        "andromeda",
        "mars",
        "venus",
        "pluto",
        "jupiter",
        "betelguese",
        "sun",
        "ceres",
        "pallas",
        "vesta",
        "hygiea",
        "europa",
        "davida",
        "sylvia",
        "cybele",
        "eunomia",
        "juno",
        "hektor",
        "foris",
        "deimos",
        "phobos",
    ];
    
    let strings: Vec<_> = strs.into_iter().map(|s| s.to_string()).collect();
    let mut rng = thread_rng();
    let markov = StringMarkov::new(5, &strings);
    
    for order in 1..=3 {
        println!("---- Order: {order}");
        for _ in 0..20 {
            println!("{:?}", markov.generate(&mut rng, order, 20));
        }
        println!()
    }
}
