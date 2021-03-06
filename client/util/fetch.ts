/**
 * Helper functions for communicating with the server's API.
 */

type Dict = { [key: string]: any }

// Fetches and decodes a JSON response from the API. Returns a tuple of the
// fetched resource and error, if any
export async function fetchJSON<T>(url: string): Promise<[T, string]> {
	const res = await fetch(url)
	if (res.status !== 200) {
		return [null, await res.text()]
	}
	return [await res.json(), ""]
}

// Avoids stale fetches from the browser cache.
export async function uncachedGET(url: string): Promise<Response> {
	const h = new Headers()
	h.append("Cache-Control", "no-cache")
	return await fetch(url, {
		method: "GET",
		credentials: "include",
		headers: h,
	})
}

// Send a POST request with a JSON body to the server.
export function postJSON(url: string, data: Dict): Promise<Response> {
	return fetch(url, {
		method: "POST",
		credentials: "include",
		body: JSON.stringify(data),
	})
}

// Send a POST multipart/form-data request to the server.
export function postForm(url: string, data: Dict): Promise<Response> {
	return fetch(url, {
		method: "POST",
		credentials: "include",
		body: toFormData(data),
	})
}

function toFormData(data: Dict): FormData {
	const form = new FormData()
	for (let k of Object.keys(data)) {
		const v = data[k]
		if (Array.isArray(v)) {
			k += "[]"
			v.forEach(item => form.append(k, item))
		} else {
			form.append(k, v)
		}
	}
	return form
}
