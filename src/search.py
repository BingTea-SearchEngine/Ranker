import requests
import pickle

with open('../data/credentials.txt', 'r') as f:
    api_key, cse_id = f.read().split()
    
with open('../data/google.pkl', 'rb') as f:
    responses = pickle.load(f)

with open('../data/searches.txt', 'r') as f:
    searches = f.read().splitlines()
    
def google_search(query, num_results=10):
    url = "https://www.googleapis.com/customsearch/v1"
    params = {
        "key": api_key,
        "cx": cse_id,
        "q": query,
        "num": num_results
    }
    response = requests.get(url, params=params)
    response.raise_for_status()  # Raises an error for bad responses
    return response.json()

for search in searches:
    if search not in responses:
        print("Searching:", search)
        response = google_search(search)
        responses[search] = response
        
with open('../data/google.pkl', 'wb') as f:
    pickle.dump(responses, f)