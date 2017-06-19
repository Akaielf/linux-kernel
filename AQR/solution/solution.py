from collections import defaultdict

def handle_fill(values, hash_t):
	if values[2] not in hash_t:
		item = {}
		item['cost'] = float(values[3]) * int(values[4])
		item['num'] = int(values[4])
		hash_t[values[2]] = item
	else:
		item = hash_t[values[2]]
		if values[5] == 'B':
			item['cost'] += float(values[3]) * int(values[4])
			item['num'] += int(values[4])
		else:
			#if int(values[4]) > item['num']:
				#return
			item['cost'] -= float(values[3]) * int(values[4])
                        item['num'] -= int(values[4])
def handle_price(values, hash_t):
	key = values[2]
	if key not in hash_t:
		print 'PNL ' + values[1] + ' ' + key + ' 0.0 0.0'
	else:
		print 'PNL ' + values[1] + ' ' + key + ' ' + str(hash_t[key]['num']) + ' ' + str(float(values[3]) * hash_t[key]['num'] - hash_t[key]['cost'])
	

def main():
	hash_t = defaultdict(dict)
	with open('fills') as f:
		fills = f.readlines()
	with open('prices') as f:
		prices = f.readlines()

	if len(fills) == 0 or len(prices) == 0: 	# handle error conditions
		return
	values_1 = fills[0].strip().split(' ')
	values_2 = prices[0].strip().split(' ')
	i, j = 1, 1
	while values_2 != None:
		if values_1 == None:
			while values_2 != None:
				handle_price(values_2, hash_t)
				if j != len(prices):
					values_2 = prices[j].strip().split(' ')
					j += 1
				else:
					values_2 = None
			break
		
		if values_2 == None or values_1[1] <= values_2[1]:
			handle_fill(values_1, hash_t)
			if i != len(fills):
				values_1 = fills[i].strip().split(' ')
				i += 1
			else:
				values_1 = None
		else:
			handle_price(values_2, hash_t)
			if j != len(prices):
				values_2 = prices[j].strip().split(' ')
				j += 1
			else:
				values_2 = None
main()			
