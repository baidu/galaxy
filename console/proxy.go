/**********************************************************
*
* wangtaize <wangtaize@baidu.com>
* Date:2015-03-27 15:24
* File:reverseproxy.go
* Copyright(C) www.baidu.com
*
**********************************************************/
package main

import (
	"bufio"
	"flag"
	"fmt"
	"net/http"
	"net/http/httputil"
	"net/url"
	"os"
	"strings"
)

type MultiHostReversProxy struct {
	Route       map[string]*httputil.ReverseProxy
	Pattern     map[string]string
	DefaultHost string
}

func New(confPath string, defaultHost string) (*MultiHostReversProxy, error) {
	pattern := ReadPattern(confPath)
	route := make(map[string]*httputil.ReverseProxy)
	for _, v := range pattern {
		targetUrl, err := url.Parse(v)
		if err != nil {
			return nil, err
		}
		rproxy := httputil.NewSingleHostReverseProxy(targetUrl)
		route[v] = rproxy
	}
	defaultUrl, err := url.Parse(defaultHost)
	if err != nil {
		return nil, err
	}
	defaultProxy := httputil.NewSingleHostReverseProxy(defaultUrl)
	route[defaultHost] = defaultProxy
	r := new(MultiHostReversProxy)
	r.Route = route
	r.Pattern = pattern
	r.DefaultHost = defaultHost
	return r, nil
}
func ReadPattern(confpath string) map[string]string {
	pattern := make(map[string]string)
	file, err := os.Open(confpath)
	if err != nil {
		return pattern
	}
	defer file.Close()
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		parts := strings.Split(scanner.Text(), " ")
		if len(parts) != 2 {
			continue
		}
		pattern[strings.TrimSpace(parts[0])] = strings.TrimSpace(parts[1])
	}
	return pattern
}
func (m *MultiHostReversProxy) Routing(w http.ResponseWriter, r *http.Request) {
	host := m.chooseRoute(r)
	if host == "" {
		fmt.Printf("error")
	}
	m.Route[host].ServeHTTP(w, r)
}

func (m *MultiHostReversProxy) chooseRoute(r *http.Request) string {
	for k, v := range m.Pattern {
		if strings.HasPrefix(r.RequestURI, k) {
			return v
		}
	}
	return m.DefaultHost
}

func main() {
	defaultHost := flag.String("host", "", "specify the default host")
	confPath := flag.String("conf", "./conf", "specify conf file path")
	port := flag.String("port", "8080", "specify port to listen")
	flag.Parse()
	proxy, _ := New(*confPath, *defaultHost)
	http.HandleFunc("/", proxy.Routing)
	http.ListenAndServe("0.0.0.0:"+*port, nil)
}
