# TCP configuration interface definition

## Connection
connect to the configuration port using TCP protocol and enter commands to configure gateway.
A command is a single liner enfing with line feed. For example:  
`command_name <options>\r`

## Command reference
**add_rest_api** -i &lt;unique api Id&gt; -n &lt;api name&gt; -d &lt;api description&gt; -p &lt;available protocols (http|https|http/https)&gt;

**add_rest_api_operation** -i &lt;unique operation id which is unique with in the api&gt; -a &lt;api id&gt; -r &lt;resource path&gt; -m &lt;http verb&gt;  
*Here &lt;resource path&gt; and &lt;http verb&gt; should be unique within the api.*

**add_rest_api_back_end_pool** -i &lt;unique id for the pool&gt; -t &lt;type of the pool (rest/soap) -p &lt;protocol to use (http/https)&gt;  
*When looking up pool while  processing a rest api request, the id for the load balancing pool is formed as follows:*  
`<api Id of the request>/<resource path of request>/<operation (GET|POST|etc)>/<deployment mode>/endpoints`

*Here 'deployment mode' refers to the deployment mode in which the gateway is run. for example a gateway running in a production
deployment may have this set to 'production'.*

*Finding an endpoint group for a given rest api request is based on most specific match of the id without deployment mode and &lt;endpoints&gt;. 
For example, assume there is an api installed with following details:*
- api id: api_1
- [operation_1]
  - `operation_id: op1, resource_path: /foo, http_verb: GET`
- [operation_2]
  - `operation_id: op2, resource_path: /foo, http_verb: PUT`
- [operation_3]
  - `operation_id: op3, resource_path: /foo/bar, http_verb: get`
- [operation_4]
  - `operation_id: op4, resource_path: /bar, http_verb: GET`
- [operation_5]
  - `operation_id: op5, resource_path: /foo, http_verb: POST`

End point id's are formed as follows:
* `for op1: api_1/foo/GET/production/endpoints`
* `for op2: api_1/foo/PUT/production/endpoints`
* `for op3: api_1/foo/bar/GET/production/endpoints`
* `for op4: api_1/bar/GET/production.endpoints`
* `for op5: api_1/foo/POST/production/endpoints`

Assume following backend pools are available:
- [pool_1]
  - `id: api_1/production/endpoints`
- [pool_2]
  - `id: api_1/foo/production/endpoints`
- [pool_3]
  - `id: api_1/foo/GET/production/endpoints`
- [pool_3]
  - `id: api_1/foo/bar/production.endpoints`

pool which will be selected are:
- **op1**: pool_3
- **op2**: pool_2
- **op3**: pool_3
- **op4**: pool_1
- **op5**: pool_2


**add_back_end** -i &lt;unique id for the server with in the pool&gt; -g &lt;pool id&gt; -h &lt;host&gt; -p &lt;port&gt;  
*Adds a server to a pool*

**add_rest_api_operation_validator** -i &lt;unique id for the validator&gt; -a &lt;api id&gt; -o &lt;operation id for which this validator applies&gt;

**add_header_validator** -i &lt;unique id for header validator&gt; -v &lt;validator to associate with&gt; -h &lt;header name&gt; -t &lt;type of the validator (compulsory|data_type)&gt; -d &lt;should be specified when type is set to data_type. data type expected.(integer|decimal|string|bool)&gt;  
*header validators are added to an existing operation validator.*

**add_path_validator** -i &lt;unique id for path validator&gt; -v &lt;validator to associate with&gt; -n &lt;position of the placeholder path argument&gt; -t &lt;type of the validator (compulsory|data_type)&gt; -d &lt;should be specified when type is set to data_type. data type expected.(integer|decimal|string|bool)&gt;  
*path validators are added to an existing operation validator.*

**add_throttler** -i &lt;unique id for the throttler&gt; -l &lt;level of the throttler. can be api|path|operation|subscription|application&gt; -t &lt;type of the throttler (fixed_window|sliding_window|sliding_log)&gt; -d &lt;width of the window&gt; -n &lt;allowed count inside the window&gt;  
*add a throttler to act on the given level. Levels are:*
- api - throttle at api level
- path - throttle at resource path level inside an api
- operation - throttle at operation level inside an api
- subscription - throttle at subscription level
- application - throttle at application level



