<input type="hidden" name="phase_05">

<h3>{t 1="6/11"}STEP %1: Confirm archived user accounts{/t}</h3>

{foreach from=$data item=row key=key}
    <table summary="{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}User account: %1, %2{/t}">
        <thead>
            <tr>
        {if isset($data[$key]['aux_accounts'])}
                <th colspan={2 + count($data[$key]['aux_accounts'])}>
        {else}
                <th colspan="2">
        {/if}
        {t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}User account: %1, %2{/t}
                </th>
            </tr>
        </thead>
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
        <tbody>
    </table>
    <br>
{/foreach}
