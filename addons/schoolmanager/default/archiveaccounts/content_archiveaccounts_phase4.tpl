{if $accounts_reviewed != TRUE}
    <input type="hidden" name="phase_04">
    <h3>{t 1="5/11"}STEP %1: Review user account objects before archiving{/t}</h3>
{/if}

{foreach from=$data item=row key=key}
    <table summary="{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}User account: %1, %2{/t}">
        <thead>
            <tr>
        {if isset($data[$key]['aux_accounts'])}
                <th colspan={2 + count($data[$key]['aux_accounts'])}>
        {else}
                <th colspan="2">
        {/if}
        {if $data[$key]['main_account']['_status'][0] === "not-found"}
                    {t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}New user account: %1, %2{/t}
        {else}
                    {t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}User account: %1, %2{/t}
        {/if}
                </th>
            </tr>
        <thead>
        <tbody>
        {if isset($data[$key]['aux_accounts'])}
            <tr>
                <td>
                    <table summary="{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}Account group for %1, %2{/t}">
        {/if}
        {foreach from=$data[$key]['main_account'] item=value key=property}
                        <tr>
                            <td>
                                <b>{$property}:</b>
                            </td>
                            <td>
            {if $property != "userPassword" || $data[$key]['main_account'][$property][0]===""}
                                    {$data[$key]['main_account'][$property][0]}
            {elseif strpos($data[$key]['main_account']['_status'][0],"exists")!==FALSE }
                                {t}<keep>{/t}
            {elseif strpos($data[$key]['main_account']['_status'][0],"data-incomplete")!==FALSE }
            {else}
                                ************
            {/if}
                            </td>
                        </tr>
        {/foreach}
        {if $accounts_reviewed && $accounts_imported && !preg_match('/ignore/', $data[$key]['main_account']['_actions'][0])}
                        <tr><td colspan="2" style="height: 0.2em;"></td></tr>
                        <tr>
                            <td>
                            <b>{t}Groups{/t}:</b>
                            </td>
                            <td>
                            <b>{t}Group membership actions{/t}:</b>
                            </td>
                        </tr>
            {foreach $data[$key]['groups'] item=group key=idx_group}
                {if in_array('posixGroup', $group['objectClass']) and in_array($group['cn'][0], array_keys($data[$key]['main_account']['_group_actions']))}
                                <tr>
                                    <td>
                                    {$group['cn'][0]}
                                    </td>
                                    <td>
                                    {$data[$key]['main_account']['_group_actions'][$group['cn'][0]]}
                                    </td>
                                </tr>
                {elseif in_array('gosaGroupOfNames', $group['objectClass']) and in_array($group['cn'][0], array_keys($data[$key]['main_account']['_ogroup_actions']))}
                                <tr>
                                    <td>
                                    {$group['cn'][0]}
                                    </td>
                                    <td>
                                    {$data[$key]['main_account']['_ogroup_actions'][$group['cn'][0]]}
                                    </td>
                                </tr>
                {/if}
            {/foreach}
            {if isset($data[$key]['optional_groups'])}
                {foreach $data[$key]['optional_groups'] item=group key=idx_group}
                    {if (strpos($group['_status'][0], 'not-found')===FALSE) and in_array('posixGroup', $group['objectClass']) and in_array($group['cn'][0], array_keys($data[$key]['main_account']['_group_actions']))}
                                    <tr>
                                        <td>
                                        {$group['cn'][0]}
                                        </td>
                                        <td>
                                        {$data[$key]['main_account']['_group_actions'][$group['cn'][0]]}
                                        </td>
                                    </tr>
                    {elseif (strpos($group['_status'][0], 'not-found')===FALSE) and in_array('gosaGroupOfNames', $group['objectClass']) and in_array($group['cn'][0], array_keys($data[$key]['main_account']['_ogroup_actions']))}
                                    <tr>
                                        <td>
                                        {$group['cn'][0]}
                                        </td>
                                        <td>
                                        {$data[$key]['main_account']['_ogroup_actions'][$group['cn'][0]]}
                                        </td>
                                    </tr>
                    {/if}
                {/foreach}
            {/if}
        {/if}
        </tbody>

    </table>
    <br>
{/foreach}

